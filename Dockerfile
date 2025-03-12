FROM archlinux

# 添加镜像源
RUN echo -e "#清华源\nServer = https://mirrors.tuna.tsinghua.edu.cn/archlinux/\$repo/os/\$arch\n#阿里源\nServer = http://mirrors.aliyun.com/archlinux/\$repo/os/\$arch\n#中科大源\nServer = https://mirrors.ustc.edu.cn/archlinux/\$repo/os/\$arch\n$(cat /etc/pacman.d/mirrorlist)" > /etc/pacman.d/mirrorlist

# 更新系统并安装软件包
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm cmake make gcc liburing tbb

# 复制项目文件
COPY . /root/co_async

# 设置工作目录
WORKDIR /root/co_async

# 启动容器时默认进入 Bash shell
CMD ["/bin/bash"]
