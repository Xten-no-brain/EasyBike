# EasyBike - 单车管理系统

![C++](https://img.shields.io/badge/C++-11-blue) 
![Protocol Buffers](https://img.shields.io/badge/Protocol_Buffers-v3.15.8-green)
![License](https://img.shields.io/badge/License-MIT-orange)

基于 C++ 实现的分布式管理系统

支持车辆调度、用户管理、数据持久化及高并发

网络通信使用 Protocol Buffers 进行高效数据序列化

---
## 项目目录结构

```
EasyBike/
├── CMakeLists.txt                # CMake 构建脚本
├── EasyBike.vcxproj              # Visual Studio 项目文件
├── EasyBike.vcxproj.filters      # Visual Studio 项目过滤器
├── EasyBike.vcxproj.user         # Visual Studio 用户项目设置
├── headers/                      # 头文件目录
│   ├── BusProcessor.h            # 业务处理器类
│   ├── configdef.h               # 配置定义
│   ├── DispatchMsgService.h      # 消息分发服务类
│   ├── events_def.h              # 事件定义
│   ├── eventtype.h               # 事件类型定义
│   ├── glo_def.h                 # 全局定义
│   ├── ievent.h                  # 事件类基类
│   ├── iEventHandler.h           # 事件处理器类
│   ├── iniconfig.h               # INI 配置文件类
│   ├── Logger.h                  # 日志系统类
│   ├── NetworkInterface.h        # 网络接口类
│   ├── sqlconnection.h           # 数据库连接类
│   ├── SqlTables.h               # 数据库表操作类
│   └── user_event_handler.h      # 用户事件处理器类
├── src/                          # 源代码目录
│   ├── DispatchMsgService.cpp    # 消息分发服务实现
│   ├── events_def.cpp            # 事件定义实现
│   ├── eventtype.cpp             # 事件类型实现
│   ├── ievent.cpp                # 事件类基类实现
│   ├── iniconfig.cpp             # INI 配置文件类实现
│   ├── Logger.cpp                # 日志系统类实现
│   ├── main.cpp                  # 主程序入口
│   ├── NetworkInterface.cpp      # 网络接口类实现
│   ├── sqlconnection.cpp         # 数据库连接类实现
│   ├── user_event_handler.cpp    # 用户事件处理器实现
│   └── user_service.cpp          # 用户服务类实现
└── bike.pb.h                     # Protocol Buffers 生成的头文件
```

## 核心类功能说明

### 1. 全局定义与工具类
- **`glo_def.h`**
  - 提供全局类型别名和宏定义，统一基础数据类型（如 `u8`、`i32` 等）和常量。
  - 定义全局状态码（如 `TRUE`、`FALSE`）和最大消息长度等。

- **`Logger.h` 和 `Logger.cpp`**
  - 实现日志系统，支持多种日志输出方式（如文件、控制台）。
  - 提供日志宏（如 `LOG_INFO`、`LOG_ERROR`），便于在代码中记录日志。

- **`iniconfig.h` 和 `iniconfig.cpp`**
  - 实现 INI 配置文件的加载和解析。
  - 提供获取配置信息的接口，支持单例模式。

### 2. 事件处理模块
- **`ievent.h` 和 `ievent.cpp`**
  - 定义事件基类 `iEvent`，包含事件 ID、序列号和序列化方法。
  - 提供事件的生成、序列化和反序列化功能。

- **`iEventHandler.h`**
  - 定义事件处理器基类 `iEventHandler`。
  - 提供事件处理的接口，子类需实现具体的事件处理逻辑。

- **`events_def.h` 和 `events_def.cpp`**
  - 定义各种具体事件类（如 `MobileCodeReqEv`、`LoginReqEv` 等）。
  - 每个事件类对应特定的客户端请求或服务器响应。
  - 实现事件的序列化和反序列化，便于网络传输。

- **`eventtype.h` 和 `eventtype.cpp`**
  - 定义全局事件类型枚举（如 `EEVENTID_GET_MOBILE_CODE_REQ`）。
  - 提供错误码与错误描述的映射，便于调试和排查问题。

### 3. 消息分发与网络模块
- **`DispatchMsgService.h` 和 `DispatchMsgService.cpp`**
  - 实现消息分发服务，负责将客户端请求解析为事件并分发给对应的处理器。
  - 提供事件订阅和取消订阅机制，支持多线程处理。
  - 管理响应事件队列，确保响应消息按顺序发送给客户端。

- **`NetworkInterface.h` 和 `NetworkInterface.cpp`**
  - 实现网络接口类，管理客户端连接和数据传输。
  - 提供网络事件的回调函数（如连接建立、数据读取、错误处理）。
  - 支持消息的分帧处理，确保完整的消息能够正确解析。

### 4. 数据库操作模块
- **`sqlconnection.h` 和 `sqlconnection.cpp`**
  - 实现数据库连接类，封装 MySQL 数据库操作。
  - 提供查询、插入、更新等基本数据库操作接口。
  - 支持自动重连和错误处理。

- **`SqlTables.h`**
  - 提供数据库表的创建和初始化功能。
  - 包括用户表、单车表、骑行记录表等的创建。
  - 插入默认管理员账户，确保系统初始可用。

- **`user_service.h` 和 `user_service.cpp`**
  - 实现用户服务类，封装用户相关的数据库操作。
  - 提供用户注册、登录、单车管理等功能。
  - 支持账户余额查询和骑行记录查询。

### 5. 业务逻辑模块
- **`user_event_handler.h` 和 `user_event_handler.cpp`**
  - 实现用户事件处理器，处理各类用户请求事件。
  - 包括获取验证码、用户注册、登录、单车管理等。
  - 调用用户服务类完成具体的数据库操作。

- **`BusProcessor.h` 和 `BusProcessor.cpp`**
  - 实现业务处理器类，负责初始化数据库结构。
  - 创建必要的数据库表，并初始化系统数据。

### 6. 入口与配置
- **`main.cpp`**
  - 程序入口，负责初始化日志系统和配置文件。
  - 启动网络接口，加载数据库连接。
  - 初始化线程池和消息分发服务，进入主事件循环。

- **`CMakeLists.txt`**
  - CMake 构建脚本，定义项目的构建规则。
  - 指定源文件、头文件和第三方依赖库。
---

## 快速开始
- **编译器**: GCC 9+ / Clang 10+ / MSVC 2019+
- **依赖库**:
  - [Protocol Buffers](https://github.com/protocolbuffers/protobuf) (v3.15+)
  - [MySQL Connector/C++](https://dev.mysql.com/downloads/connector/cpp/) (v8.0+)
- **数据库**: MySQL 8.0+
```bash
git clone https://github.com/Xten-no-brain/EasyBike.git
cd shared_bike
```
