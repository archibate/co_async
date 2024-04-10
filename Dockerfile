FROM archlinux
RUN pacman -Syu --noconfirm && pacman -S --noconfirm cmake make gcc liburing
COPY . /root/co_async
WORKDIR /root/co_async
CMD ["/bin/bash"]
