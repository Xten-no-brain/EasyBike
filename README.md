# EasyBike - 单车管理系统

![C++](https://img.shields.io/badge/C++-11-blue) 
![Protocol Buffers](https://img.shields.io/badge/Protocol_Buffers-v3.15.8-green)
![License](https://img.shields.io/badge/License-MIT-orange)

基于 C++ 实现的分布式管理系统

支持车辆调度、用户管理、数据持久化及高并发

网络通信使用 Protocol Buffers 进行高效数据序列化

---

## 功能特性

- **用户管理**

  用户通过手机验证码进行注册和登录

- **单车管理**

  支持增减单车数量，更新单车状态和位置信息，确保单车数据的实时性和准确性

- **支付管理**

  根据用户的骑行时间和距离计算费用，支持在线支付功能，管理用户的支付记录和账户余额

- **网络通信**  

  基于 Libevent 的通信框架，支持多线程高并发请求处理

- **数据持久化**  

  使用 MySQL 存储用户及车辆数据，集成连接池优化性能

- **日志系统**  

  记录操作日志及错误信息，支持调试

---

## 快速开始

### 前置条件

- **编译器**: GCC 9+ / Clang 10+ / MSVC 2019+
- **依赖库**:
  - [Protocol Buffers](https://github.com/protocolbuffers/protobuf) (v3.15+)
  - [MySQL Connector/C++](https://dev.mysql.com/downloads/connector/cpp/) (v8.0+)
- **数据库**: MySQL 8.0+

### 克隆仓库

```bash
git clone https://github.com/Xten-no-brain/EasyBike.git
cd shared_bike

```
