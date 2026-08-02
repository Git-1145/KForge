# KForge

> 一个跨平台 C++ 基础工具库与算法学习项目。

## 简介

KForge 是一个个人 C++ 学习项目，作为算法与 C++ 特性学习的实验场。

## 项目结构

```
kForge/
├── base/              # 核心基础库
│   ├── KF.hpp         # 模块声明与公共接口
│   ├── FIO.cpp        # 自定义配置格式解析器（Document 类）
│   ├── CLI.cpp        # 控制台交互封装
│   ├── KTIMER.cpp     # 高精度计时器
│   ├── UTILITY.cpp    # 通用工具函数
│   └── references.md  # API 使用手册
├── test/              # 模块测试与系统测试
│   ├── cfg_test.txt   # 复杂边界测试配置
│   └── system_test.cpp # 自动化系统测试框架
├── study/             # 算法与 C++ 特性学习代码
│   ├── algorithm/     # 排序、搜索算法
│   └── cpp/           # C++ 进阶特性实验
├── config/            # 配置模板
├── exp/               # 实验/扩展（预留）
├── notes/             # 笔记（预留）
└── projects/          # 项目（预留）
```

## API 文档

详见 [`base/references.md`](base/references.md)。

## 变更日志

## 贡献与许可

本项目为个人学习项目，持续迭代中。
