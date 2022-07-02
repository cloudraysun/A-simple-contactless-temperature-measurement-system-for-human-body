import sensor#
import image
import lcd
import KPU as kpu
import time
from Maix import FPIOA, GPIO
import gc
from fpioa_manager import fm
from board import board_info
import utime
import os
import uos
import ubinascii

task_fd = kpu.load(0x200000) # 从flash 0x200000 加载人脸检测模型
task_ld = kpu.load(0x300000) # 从flash 0x300000 加载人脸五点关键点检测模型
task_fe = kpu.load(0x400000) # 从flash 0x400000 加载人脸196维特征值模型

clock = time.clock()#时钟初始化

fm.register(board_info.BOOT_KEY, fm.fpioa.GPIOHS0)#按键初始化
key_gpio = GPIO(GPIO.GPIOHS0, GPIO.IN)
start_processing = False

BOUNCE_PROTECTION = 50 #保护时间


def set_key_state(*_): #读取按键状态
    global start_processing
    start_processing = True
    utime.sleep_ms(BOUNCE_PROTECTION)

#配置一个中断处理程序，当 pin 的触发源处于活动状态时调用它。如果管脚模式为 pin.in，则触发源是管脚上的外部值。
key_gpio.irq(set_key_state, GPIO.IRQ_RISING, GPIO.WAKEUP_NOT_SUPPORT)

lcd.init() #lcd初始化
sensor.reset() #重置与初始化
sensor.set_pixformat(sensor.RGB565) #设置帧格式
sensor.set_framesize(sensor.QVGA) #设置帧大小
sensor.set_hmirror(1) #设置水平镜像
sensor.set_vflip(1) #设置摄像头垂直翻转
sensor.run(1)#图像捕捉控制
anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437,
          6.92275, 6.718375, 9.01025)  #anchor for face detect 用于人脸检测的Anchor
dst_point = [(44, 59), (84, 59), (64, 82), (47, 105),
             (81, 105)]  #standard face key point position 标准正脸的5关键点坐标 分别为 左眼 右眼 鼻子 左嘴角 右嘴角
a = kpu.init_yolo2(task_fd, 0.5, 0.3, 5, anchor) #为yolo2网络模型传入初始化参数， 只有使用yolo2时使用
img_lcd = image.Image() #创建Image类
img_face = image.Image(size=(128, 128)) #设置 128 * 128 人脸图片buf
a = img_face.pix_to_ai() #更新到rgb888区域
record_ftr = [] #空列表 用于存储当前196维特征
record_ftrs = [] #空列表 用于存储按键记录下人脸特征， 可以将特征以txt等文件形式保存到sd卡后，读取到此列表，即可实现人脸断电存储。
names = ['SYH', 'Mr.2', 'Mr.3', 'Mr.4', 'Mr.5',
         'Mr.6', 'Mr.7', 'Mr.8', 'Mr.9', 'Mr.10'] #人名标签，与上面列表特征值一一对应。

ACCURACY = 85 #人脸确认标准
reco = ''
record = []
#存储特征值到flash/features.txt下
def save_feature(feat):
    with open("/flash/features.txt", "a") as f:
        record =ubinascii.b2a_base64(feat)
        f.write(record)

# print(record_ftr,names)

# print(len(record_ftr))
# print("end")

while (1): #主循环
    img = sensor.snapshot() #从摄像头获取一张图片
    clock.tick() #记录时刻，用于计算帧率
    code = kpu.run_yolo2(task_fd, img) # 运行人脸检测模型，获取人脸坐标位置
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
                    names[index], max_score)), color=(0, 255, 0), scale=2)
            else:
                a = img.draw_string(i.x(), i.y(), ("X :%2.1f" % (
                    max_score)), color=(255, 0, 0), scale=2)
            if start_processing: #如果按下按键，则将捕捉的人脸特征值临时添加
                record_ftr = feature #记录人脸特征值
                record_ftrs.append(record_ftr) #添加进人脸特征值序列
                save_feature(record_ftr) #保存特征值在flash下
                a = img.draw_string(100,100, "Stor successful", color=(0,255,0),scale=2)
                utime.sleep_ms(1000)
                start_processing = False #重置按键状态

            break
    fps = clock.fps() #计算帧率
    print("%2.1f fps" % fps)#显示帧率
    a = lcd.display(img)
    gc.collect()
    # kpu.memtest()#打印内存使用情况，包括GC内存和系统堆内存

#a = kpu.deinit(task_fe) #反初始化，进行切换模型时，需要先运行
#a = kpu.deinit(task_ld)
#a = kpu.deinit(task_fd)
