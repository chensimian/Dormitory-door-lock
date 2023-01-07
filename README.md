# Dormitory-door-lock
代码在另外一个分支 masterv<br />
APP不开源<br />
[宿舍实际演示](https://www.bilibili.com/video/BV1Dr4y117cD/?vd_source=113ed443fffce56dcbb3fa5714926d19)<br />
- 采用STM32作为主控芯片，ESSP32_CAM作为wifi与摄像监控，LCD12864液晶显示屏作为显示，AS608指纹识别模块、继电器、WIFI模块<br />
- 具有三种解锁方式：指纹解锁、密码解锁、APP解锁<br />
- 解锁时密码或者指纹错误液晶显示屏会提示出错，3次错误就会锁定，需要等待1分钟才会解锁。输入正确密码和指纹时继电器吸合，电磁锁打开，几秒后自动断开<br />
- 有矩形键盘，可以进入管理系统，对指密码进行录入或者删除，输入密码有退格退格键和重输键，方便输入错误时可退格或者重输<br />
- 将系统议接入ONEnet云平台，获取数据。还可每个人的开门记录下来，并附带时间编号，将ESP32_抓拍的照片上传到onenet云平台<br />
