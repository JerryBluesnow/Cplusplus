# 从Linux内核代码中学习获得结构体成员偏移量的方法

    这个宏定义很好解释：将0地址强制转换为TYPE类型指针，并取得MEMBER，然后获取该MEMBER地址，最后将该地址强制转换为表示大小的整数。该整数即为该成员的偏移量。