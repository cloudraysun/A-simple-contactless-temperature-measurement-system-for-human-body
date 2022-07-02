import sensor,image,lcd  # import 相关库
import KPU as kpu
import time
from Maix import FPIOA,GPIO
from fpioa_manager import fm
import ubinascii
from machine import UART
from machine import Timer
import machine
import time
import utime
from board import board_info
import os
import json
import random

###################################################
#人脸识别部分
###################################################
name_number=0 #用于临时存储的姓名编号
record_ftrs = [] #空列表 用于存储按键记录下人脸特征， 可以将特征以txt等文件形式保存到sd卡后，读取到此列表，即可实现人脸断电存储。
names = [] #运行时人名标签，与上面列表特征值一一对应。
name_transfer = [] #用于传输的列表
face_transfer = []
def face_read():
    #读取flash中feature的数据
    global record_ftrs
    global name_transfer
    global face_transfer
    feature_file_exists = 0
    for v in os.ilistdir('/flash'):#
        if v[0] == 'feature.json' and v[1] == 0x8000:#0x8000 is file
            feature_file_exists = 1
            if(feature_file_exists):
                #print("start")
                with open('/flash/feature.json','r') as f:
                    for row in f.readlines(): #读取出来的是字符串格式
                        # 将字符串转化为原本格式
                        row = json.loads(row)
                        #row[0]为编号（为字符串格式），row[1]为特征值
                        name_transfer.append(str(row[0]))
                        face_transfer.append(row[1])
                        names.append(row[0])#添加编号
                        record_ftrs.append(bytearray(ubinascii.a2b_base64(row[1].encode('gkb'))))#添加特征值二进制数组格式

def face_check():
    global check_times#检查次数
    global save
    global switch
    global start_processing
    global name_number
    global transfer
    global face_pass
    global mask_pass
    global test #调试模式
    global delete_number #删除编号
    global record_ftrs #面部特征值
    global names #名字
    check_times = 0#每次切换都要初始化
    task_fd = kpu.load(0x200000) # 从flash 0x200000 加载人脸检测模型
    task_ld = kpu.load(0x300000) # 从flash 0x300000 加载人脸五点关键点检测模型
    task_fe = kpu.load(0x400000) # 从flash 0x400000 加载人脸196维特征值模型

    anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437,
              6.92275, 6.718375, 9.01025)  #anchor for face detect 用于人脸检测的Anchor
    dst_point = [(44, 59), (84, 59), (64, 82), (47, 105),
                 (81, 105)]  #standard face key point position 标准正脸的5关键点坐标 分别为 左眼 右眼 鼻子 左嘴角 右嘴角
    a = kpu.init_yolo2(task_fd, 0.5, 0.3, 5, anchor) #为yolo2网络模型传入初始化参数， 只有使用yolo2时使用
    img_lcd = image.Image() #创建Image类
    img_face = image.Image(size=(128, 128)) #设置 128 * 128 人脸图片buf
    a = img_face.pix_to_ai() #更新到rgb888区域
    record_ftr = [] #空列表 用于存储当前196维特征

    ACCURACY = 85 #人脸确认标准

    clock = time.clock()#时钟初始化
    while (1): #主循环
        tim.start()  #定时器中断开始，必须放在这里
        if switch!=1:#切换模型，释放资源
            a = kpu.deinit(task_fe) #反初始化，进行切换模型时，需要先运行
            a = kpu.deinit(task_ld)
            a = kpu.deinit(task_fd)
            break
        if transfer==1: #传送数据
            i = len(name_transfer)
            for j in range(0,i):
                uart1.write(name_transfer[j])
                uart1.write(face_transfer[j])
            transfer = 0
        #if(delete==1): #删除编号
            #for i in range(len(names)): #在编号序列搜索相同的
                #if names[i]==delete_number:
                    ##运行时的特征值库
                    #names.pop(i) #去除对应的编号
                    #record_ftrs.pop(i)#去除对应的特征值
                    #break
            #delete=0
        img = sensor.snapshot() #从摄像头获取一张图片
        clock.tick() #记录时刻，用于计算帧率
        code = kpu.run_yolo2(task_fd, img) # 运行人脸检测模型，获取人脸坐标位置
        if test==1: #如果在调试模式下，不接收STM32端数据
            data_stm32=None
            img.draw_string(250,0, "TEST", color=(255,0,0),scale=2)
        else:
            img.draw_string(250,0, "FORMAL", color=(0,255,0),scale=2)
        if code:
            for i in code:
                # Cut face and resize to 128x128
                a = img.draw_rectangle(i.rect()) #在屏幕显示人脸方框
                face_cut = img.cut(i.x(), i.y(), i.w(), i.h()) # 裁剪人脸部分图片到 face_cut
                face_cut_128 = face_cut.resize(128, 128) # 将裁出的人脸图片 缩放到128 * 128像素
                a = face_cut_128.pix_to_ai() # 将猜出图片转换为kpu接受的格式
                # a = img.draw_image(face_cut_128, (0,0))
                # Landmark for face 5 points
                fmap = kpu.forward(task_ld, face_cut_128) # 运行人脸5点关键点检测模型
                plist = fmap[:] # 获取关键点预测结果
                le=(i.x()+int(plist[0]*i.w() - 10), i.y()+int(plist[1]*i.h())) # 计算左眼位置， 这里在w方向-10 用来补偿模型转换带来的精度损失
                re=(i.x()+int(plist[2]*i.w()), i.y()+int(plist[3]*i.h())) # 计算右眼位置
                nose=(i.x()+int(plist[4]*i.w()), i.y()+int(plist[5]*i.h())) #计算鼻子位置
                lm=(i.x()+int(plist[6]*i.w()), i.y()+int(plist[7]*i.h())) #计算左嘴角位置
                rm=(i.x()+int(plist[8]*i.w()), i.y()+int(plist[9]*i.h())) #右嘴角位置

                a = img.draw_circle(le[0], le[1], 4)
                a = img.draw_circle(re[0], re[1], 4)
                a = img.draw_circle(nose[0], nose[1], 4)
                a = img.draw_circle(lm[0], lm[1], 4)
                a = img.draw_circle(rm[0], rm[1], 4) # 在相应位置处画小圆圈
                # align face to standard position
                src_point = [le, re, nose, lm, rm] # 图片中 5 坐标的位置
                T=image.get_affine_transform(src_point, dst_point) # 根据获得的5点坐标与标准正脸坐标获取仿射变换矩阵
                a=image.warp_affine_ai(img, img_face, T) #对原始图片人脸图片进行仿射变换，变换为正脸图像
                a=img_face.ai_to_pix() # 将正脸图像转为kpu格式
                #a = img.draw_image(img_face, (128,0))
                del(face_cut_128) # 释放裁剪人脸部分图片
                # calculate face feature vector
                fmap = kpu.forward(task_fe, img_face) # 计算正脸图片的196维特征值
                feature=kpu.face_encode(fmap[:]) #获取计算结果
                reg_flag = False
                scores = [] # 存储特征比对分数
                for j in range(len(record_ftrs)): #迭代已存特征值
                    score = kpu.face_compare(record_ftrs[j], feature) #计算当前人脸特征值与已存特征值的分数
                    scores.append(score) #添加分数总表
                max_score = 0
                index = 0
                for k in range(len(scores)): #迭代所有比对分数，找到最大分数和索引值
                    if max_score < scores[k]:
                        max_score = scores[k]
                        index = k
                if max_score > ACCURACY: #如果大于accuracy，则认定为同一个人
                    a = img.draw_string(i.x(), i.y(), ("%s :%2.1f" % (
                    names[index], max_score)), color=(0, 255, 0), scale=2) #显示名字和最大相似度
                    check_times += 1 #每识别到一次，检查次数加一
                    if(check_times>5):
                        check_times=0
                        uart1.write('a') #人脸识别通过
                        uart2.write('a')
                    #uart1.write(str(names[index]))
                else:
                    a = img.draw_string(i.x(), i.y(), ("X :%2.1f" % (
                        max_score)), color=(255, 0, 0), scale=2)
                if save == 1: #临时存储
                    record_ftr = feature #记录人脸特征值
                    record_ftrs.append(record_ftr) #添加进人脸特征值序列
                    names.append('Mr.'+str(name_number)) #临时人脸编号
                    name_number = name_number+1
                    a = img.draw_string(0,100, "Temporary stor successful", color=(0,255,0),scale=2)
                    utime.sleep_ms(1000)
                    save = 0
                elif save == 2: #存储到flash
                    name_flash=[]
                    random.seed(random.randint(0,100000))#初始化随机,5位数随机值
                    old = set(names) #创建一个集合,便于去重
                    record_ftr = feature #记录人脸特征值
                    record_ftrs.append(record_ftr) #添加进人脸特征值序列
                    if len(names)<=len(old): #保证唯一性,系统内部唯一编号，用于存储的列表
                        t = random.randint(10000,100000)#生成随机编号
                        old.add(t)#添加进，去重集合中
                    names.append(t) #运行时人名标签
                    name_flash.append(t) #存储人名

                    #print(record_ftr) #bytearray格式，字符数组
                    #print('*'*10)
                    record =ubinascii.b2a_base64(record_ftr) #转化为bytes格式
                    print(record)#转化后的格式
                    name_flash.append(str(record)[2:-3])
                    #name_flash.append(str(record))

                    with open('/flash/feature.json', 'a') as fp: #存储，内置3m可存储大小，大概11000个左右
                        line=json.dumps(name_flash)
                        fp.write(line+'\n')#加换行符
                    face_read() #重新读取数据
                    a = img.draw_string(0,100, "Flash has stored", color=(0,255,0),scale=2)
                    utime.sleep_ms(1000)
                    save = 0
        if start_processing: #如果按下按键，则清除flash里的内容，但不清除临时存储的
            with open("/flash/feature.json", "w") as f:
                f.write('')
            a = img.draw_string(0,100, "Clear successful", color=(0,255,0),scale=2)
            utime.sleep_ms(1000)
            start_processing = False #重置按键状态

        save=0 #存储标志位置零
        a = img.draw_string(0,200,"face", color=(0,0,255),scale=2)
        fps =clock.fps() #计算帧率
        print("%2.1f fps"%fps) #打印帧率
        _ = lcd.display(img)

def mask_check():
    global check_times #检查次数
    global save #存储标志位
    global switch
    global start_processing
    global transfer
    check_times = 0
    color_R = (255, 0, 0)
    color_G = (0, 255, 0)
    color_B = (0, 0, 255)
    class_IDs = ['no_mask', 'mask']

    def drawConfidenceText(image, rol, classid, value): #画框
        text = ""
        _confidence = int(value * 100)

        if classid == 1:
            text = 'mask: ' + str(_confidence) + '%'
            color_text=color_G
        else:
            text = 'no_mask: ' + str(_confidence) + '%'
            color_text=color_R
        image.draw_string(rol[0], rol[1], text, color=color_text, scale=2.5)

    mask = kpu.load(0x650000)
    anchor = (0.1606, 0.3562, 0.4712, 0.9568, 0.9877, 1.9108, 1.8761, 3.5310, 3.4423, 5.6823)
    _ = kpu.init_yolo2(mask, 0.5, 0.3, 5, anchor)
    img_lcd = image.Image()

    clock = time.clock()#时钟初始化
    while (1):
        tim.start()  #定时器中断开始
        save=0 #防止切换到face时出错
        transfer=0
        if switch!=2: #模型切换，释放资源
            _ = kpu.deinit(mask) #反初始化
            break
        img = sensor.snapshot()
        code = kpu.run_yolo2(mask, img)
        clock.tick() #记录时刻，用于计算帧率
        if test==1: #如果在调试模式下，不接收STM32端数据
            data_stm32=None
            img.draw_string(250,0, "TEST", color=(255,0,0),scale=2)
        else:
            img.draw_string(250,0, "FORMAL", color=(0,255,0),scale=2)
        if code:
            totalRes = len(code)

            for item in code:
                confidence = float(item.value())
                itemROL = item.rect()
                classID = int(item.classid())

                if confidence < 0.52:
                    _ = img.draw_rectangle(itemROL, color=color_B, tickness=5)
                    continue

                if classID == 1 and confidence > 0.65:
                    _ = img.draw_rectangle(itemROL, color_G, tickness=5)
                    if totalRes == 1:
                        drawConfidenceText(img, (0, 0), 1, confidence)
                        check_times += 1
                        if(check_times>5):
                            check_times = 0
                            uart1.write('b')#口罩识别通过
                            uart2.write('b')#口罩识别通过

                else:
                    _ = img.draw_rectangle(itemROL, color=color_R, tickness=5)
                    if totalRes == 1:
                        drawConfidenceText(img, (0, 0), 0, confidence)
        fps =clock.fps() #计算帧率
        print("%2.1f fps"%fps) #打印帧率
        a = img.draw_string(0,200,"mask", color=(0,0,255),scale=2)
        _ = lcd.display(img)


###################################################
#串口1配置
###################################################
fm.register(10,fm.fpioa.UART1_TX)#串口引脚映射
fm.register(11,fm.fpioa.UART1_RX)

uart1 = UART(UART.UART1, 9600, timeout=200, read_buf_len=4096)#构建串口对象

###################################################
#串口2配置
###################################################
fm.register(6,fm.fpioa.UART2_TX)#串口引脚映射
fm.register(7,fm.fpioa.UART2_RX)

uart2 = UART(UART.UART2, 9600, timeout=200, read_buf_len=4096)#构建串口对象

###################################################
#按键设置部分
###################################################
fm.register(board_info.BOOT_KEY, fm.fpioa.GPIOHS0)#按键初始化
key_gpio = GPIO(GPIO.GPIOHS0, GPIO.IN)
start_processing = False
BOUNCE_PROTECTION=50 #保持时间
def set_key_state(*_): #读取按键状态
    global start_processing
    start_processing = True
    utime.sleep_ms(BOUNCE_PROTECTION)
#配置一个中断处理程序，当 pin 的触发源处于活动状态时调用它。如果管脚模式为 pin.in，则触发源是管脚上的外部值。
key_gpio.irq(set_key_state, GPIO.IRQ_RISING, GPIO.WAKEUP_NOT_SUPPORT)

###################################################
#定时器中断回调函数
###################################################
#全局变量
save = 0   #1：临时存储，2：存储到flash
switch = 1#切换,1人脸识别，2：口罩识别，（3：健康码）
transfer = 0 #初始传输
face_pass = 0 #人脸通过
mask_pass = 0 #口罩通过
# check_time_init=0 #超时检查次数清除
check_times=0 #累加检查次数
test=0 #调试模式
delete_number='00000' #删除编号
def on_timer(timer):  #回调函数
    global switch
    global save
    global transfer
    global check_times
    global check_time_init
    global test #调试模式
    global delete_number #删除编号
    data = []
    data_stm32 = []
    data = uart1.read(2)#手机APP操控，完整命令
    data_stm32 = uart2.read(2) #STM32操控，支持人脸和口罩切换和反馈

    #LED_R.value(1)
    # if(check_times>0): #如果识别了人脸或戴了口罩
    #     check_time_init += 1 #每500ms加一次
    #     if(check_time_init>10): #5s都没有下一次有效识别
    #         check_times = 0
    #         check_time_init = 0

    if data!=None: #手机操控
        if data == b'A':#人脸识别
            switch  = 1
            uart1.write(str(switch))
        elif data == b'B':#口罩识别
            switch = 2
            uart1.write(str(switch))
        elif data == b'D': #临时存储
            save = 1
        elif data == b'E': #存储Flash
            save = 2
        elif data == b'F': #软重启
            machine.reset() #复位重启
            # transfer = 1 #传输数据成功#传输数据
        elif data == b'G': #调试模式
            test=1
            uart2.write('t')
        elif data == b'H':
            test=0
            uart2.write('f')
    if test==1: #如果在调试模式下，不接收STM32端数据
        data_stm32=None
    if data_stm32!=None:
        if data_stm32 == b'A':#人脸识别
            switch = 1
            uart2.write(str(switch))
        elif data_stm32 == b'B':#口罩识别
            switch = 2
        #uart1.write(data)#可以尝试直接发送长串字符，即特征值
        #uart1.write(str(switch))
tim = Timer(Timer.TIMER0, Timer.CHANNEL0, mode=Timer.MODE_ONE_SHOT, period=50,
                     unit=Timer.UNIT_MS,callback=on_timer, arg=on_timer,start=False) #50ms
###################################################
#主函数
###################################################
def main():
    global switch
    global save
    global check_times
    lcd.init() #lcd初始化
    sensor.reset() #重置与初始化
    sensor.set_pixformat(sensor.RGB565) #设置帧格式
    sensor.set_framesize(sensor.QVGA) #设置帧大小
    sensor.set_hmirror(1) #设置水平镜像
    sensor.set_vflip(1) #设置摄像头垂直翻转
    sensor.run(1)#图像捕捉控制
    face_read() #读取Flash数据
    uart1.write('b')#
    uart2.write('b')#防止口罩识别的错误
    while(1):
        if switch==1:
            face_check()
        elif switch==2:
            mask_check()
main()
