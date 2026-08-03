# Changelog
所有重要的项目变更都会记录在此文件。
格式遵循 [Keep a Changelog](https://keepachangelog.com/zh-CN/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。
## 2026-08-03
### Added
- 增加 `KLOGGER.cpp` 实现文件
- 增加 `KFIO.cpp` 实现文件
### Changed
- 修改 `KSON.cpp` 实现文件
    - 增加 `Preprocess()`函数 [测试](test/KFIO.cpp)
## 2026-08-02
### Added
- 增加 `KF.hpp` 声明文件 
- 增加 `KSON.cpp` 实现文件 **开发进行中 结构需重构**
### Deprecated
- 废弃原来的 `base/`下所有文件 放入`base/trash`下，将来随时可能删除