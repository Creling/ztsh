## 项目架构说明
### 1. core
由c语言编写，代码受[orangetw/tsh](https://github.com/orangetw/tsh)启发，负责master和slave的通信
### 2. web_panel
以django框架编写，负责浏览器与master的通信

```
   browser -->   master   <----slave 1
        |---->            <----slave 2
        |---->            <----slave 2
```

## Feature
1. 支持通过浏览器不同标签页同时操作不同slave
2. 浏览器页面的刷新/关闭不影响master与slave的连接
3. 不需要slave具有公网ip
4. 支持密码登录/访问控制

## 使用指南
### 1. 环境与依赖
```
Centos 7
Python            3.7.2
channels          2.2.0    
channels-redis    2.4.0 
Django            2.1.8    
django-channels   0.7.0    
django-redis      4.10.0   
libhiredis       (c library）

【此处列举了作者的开发环境，实际安装过程中对于各模块的版本要求并不严苛】
```
### 2. 编译core中文件
1. 将`ztsh.h`中`CONNECT_BACK_HOST`项修改为`master`的ip
2. 确保有足够的权限访问`INFILEPATH`项指向的路径，且该项的值与`web_panel/revShell/consumers.py`中`unix_socket_path`一致（默认值即为为一致）
3. 执行
```
gcc -O -W -Wall -o ztsh write_log.c socket_wrapper.c ztsh.c -L`pwd` -l:libhiredis.a -lpthread
gcc -O -W -Wall -o ztshd ztshd.c write_log.c socket_wrapper.c -lutil
```
### 3. 执行 ztsh/ztshd
将ztsh在master上执行，将ztshd在slave上执行，观察日志输出
### 4. 启动Django App
1. 取消注释`ztsh/web_panel/templates/login.html`第36行注册账户按钮
2. 进入`ztsh/web_panel`目录，`python3 manage.py runserver 0:8080`
【注册后记得重新注释上注册账号按钮或干脆删去dj中处理账户注册的函数】

