FROM ros:melodic
RUN sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
RUN apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

RUN apt-get update && apt-get install -y ros-melodic-desktop-full \
		wget git nano python-rosinstall python-rosinstall-generator python-wstool build-essential

SHELL ["/bin/bash","-c"]

RUN mkdir -p /catkin_ws/src/robot-client \
    && cd /catkin_ws/src \
    && source "/opt/ros/melodic/setup.bash" \
    && catkin_init_workspace

RUN cd /catkin_ws/src \
    && git clone https://github.com/amor-ros-pkg/rosaria.git

COPY resources /
RUN dpkg -i libaria_2.9.1+ubuntu16_amd64.deb 
RUN dpkg -i mobilesim_0.9.8+ubuntu16_amd64.deb

RUN cd /catkin_ws/ \
    && source "/opt/ros/melodic/setup.bash" \
    && catkin_make

COPY ros_entrypoint.sh /ros_entrypoint.sh 
RUN chmod +x /ros_entrypoint.sh 
ENTRYPOINT ["/ros_entrypoint.sh"]

CMD ["bash"]
