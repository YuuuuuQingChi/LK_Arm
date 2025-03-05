# LK_Arm
RoboMaster Control System based on ROS2.
1. Set up Docker's apt repository.
# Add Docker's official GPG key:
```C++
sudo apt-get update
sudo apt-get install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc
```
或者
```
sudo apt-get update &&
sudo apt-get -y install ca-certificates curl &&
sudo install -m 0755 -d /etc/apt/keyrings &&
sudo curl -fsSL https://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc &&
sudo chmod a+r /etc/apt/keyrings/docker.asc &&
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://mirrors.aliyun.com/docker-ce/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null &&
sudo apt-get update &&
sudo apt-get -y install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin &&
sudo usermod -aG docker $USER
```

# Add the repository to Apt sources:
```
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
```

If you use an Ubuntu derivative distribution, such as Linux Mint, you may need to use UBUNTU_CODENAME instead of VERSION_CODENAME.

2. Install the Docker packages.
```
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```
由于Docker运行默认需要root权限，我们想让普通用户也可以运行，可以参考官方文档将普通用户添加进用户组。
```
#To create the docker group and add your user:
#Create the docker group.
sudo groupadd docker
#Add your user to the docker group.
sudo usermod -aG docker $USER
```
3. Vertify
```
docker
```
4. pull
```
sudo docker pull qzhhhi/rmcs-develop:latest
```

May Need：
```
sudo mkdir -p /etc/systemd/system/docker.service.d &&
sudo tee /etc/systemd/system/docker.service.d/proxy.conf <<EOF
[Service]
Environment="HTTP_PROXY=http://127.0.0.1:7890/"
Environment="HTTPS_PROXY=http://127.0.0.1:7890/"
EOF
sudo systemctl daemon-reload &&
sudo systemctl restart docker
```
5. clone
```
git clone --recurse-submodules https://github.com/Alliance-Algorithm/RMCS.git
```

cd /workspaces/rmcs

# 添加ros2的环境变量到当前终端中
```
source /opt/ros/humble/setup.zsh
```
# ros2包的依赖自动安装
cd /path/to/develop_ws/
```
sudo rosdep install --from-paths src --ignore-src -r -y
sudo apt-get install libusb-1.0-0-dev
```
# ros2包的构建
cd /path/to/develop_ws/
```
colcon build --merge-install
```
# 加载ros2包的环境变量
cd /path/to/develop_ws/
```
source ./install/setup.zsh
```
# 启动！
ros2 launch rmcs_bringup rmcs.launch.py config:=engineer.yaml   

