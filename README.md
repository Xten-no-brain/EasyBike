# ShareBike - 共享单车管理系统（C++ 实现）

![C++](https://img.shields.io/badge/C++-17%2B-blue) 
![Protocol Buffers](https://img.shields.io/badge/Protocol_Buffers-v3.15.8-green)
![License](https://img.shields.io/badge/License-MIT-orange)

一个基于 C++ 实现的分布式共享单车管理系统，支持车辆调度、用户管理、数据持久化及高并发网络通信，使用 Protocol Buffers 进行高效数据序列化。

---

## 功能特性
- **用户管理**  
  用户注册、登录、余额查询及租车记录管理。
- **车辆调度**  
  实时监控车辆状态（空闲/使用中/故障），动态分配车辆资源。
- **网络通信**  
  基于 TCP/IP 的异步通信框架，支持多线程高并发请求处理。
- **数据持久化**  
  使用 MySQL 存储用户及车辆数据，集成连接池优化性能。
- **日志系统**  
  记录操作日志及错误信息，支持调试与审计。

---

## 快速开始

### 前置条件
- **编译器**: GCC 9+ / Clang 10+ / MSVC 2019+
- **依赖库**:
  - [Protocol Buffers](https://github.com/protocolbuffers/protobuf) (v3.15+)
  - [MySQL Connector/C++](https://dev.mysql.com/downloads/connector/cpp/) (v8.0+)
- **数据库**: MySQL 8.0+

### 1. 克隆仓库
```bash
git clone https://github.com/Xten-no-brain/ShareBike.git
cd ShareBike/shared_bike
