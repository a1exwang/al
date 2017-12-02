# AL Language Documentation

1. 基本设计
    1. 设计目标
        1. 基本语法 + 语法糖
            - 基本语法简洁， 快速上手。在基本语法上添加语法糖增加可读性。
            - 与 Clojure 对比。
        2. 元编程
            - 可以访问修改运行时 Metadata，支持元编程，使得库的使用简洁优雅。
            - 与 Ruby 对比。
        3. 强调性能
            - 对于 CPU 密集型程序性能强于传统脚本语言 Python/Ruby/JavaScript。
        4. 强调开发效率和可读性
            - AL 可能不是一个纯函数式语言，是否采用函数式特性的决定在于开发效率和可读性。
        5. 基于 LLVM 平台。
            - 不考虑兼容其他平台。LLVM 平台足够强大，减少其他平台兼容的负担。
    1. 对现代语言特性的考虑
        1. 编译？解释？
            - 基本上是解释执行。但尽可能多的编译以优化性能。
        1. 数据类型
            - (*)基本 BigInt，double，String，List，Hash，Function
        1. 面向对象？
            - 封装。Module 级封装，Module 有 private 数据从而实现隐藏细节。
            - 继承。不会支持。继承的本质是复用代码，可以通过元编程的方式实现。
            - 多态。运行时多态。由 Pattern Matching（模式匹配）支持。
        1. 内存管理
            - (*)利用 LLVM 的垃圾回收。
        1. 并行计算模型
            - (*)不对用户暴露线程概念。使用 Task 概念，每个 Task 是一个并行计算单元，作为一个 IO 任务来处理。
        1. 并发 IO 模型
            - (*)事件驱动模型。类似 JavaScript。
        1. Packaging
            - 三级抽象。函数(function)，模块(module)，包(package)。
        1. 符号查找
            - Callback
        1. Pattern Matching
    1. 语法
        - 字面值语法
            - Integer 123
            - Floating Pointer 123.45
            - String "abc", 'Alex "Haha" Wang'
            - Symbol function, fn-name, +-*/ ...(All symbols except for parenthesis)
            - List ("item1" "item2"), () for empty list
        - 类 LISP 语法
            - 函数调用. （fn-name "param1" ("a" "b"))
