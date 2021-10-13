# 异步日志

## 环境

系统: OpenSUSE 15.3  
内核: 5.3.18  
内存: 16G  
磁盘: 512G SSD  
编译选项: Cmake Release 模式(-O3)

## 说明

- 采用多缓冲设计(初始状态为双缓存)
- 双队列, 写磁盘时无需加锁
- 日志量少时可定时写磁盘(默认3s)
- `FATAL` 级日志自动刷新缓存到磁盘后停止程序
- 如果没有启动异步日志, 默认的 log 处理类会将其打印到标准输出

## 性能

### 单线程

```plain
write to cache used 1464 ms, 341.53w log per second
writeback to disk used 2413 ms, 207.21w log per second
```

### 多线程

```plain
write to cache used 10696 ms, 373.97w log per second
writeback to disk used 13023 ms, 307.15w log per second
```

多线程优化后面有时间再做, 目前性能够用.

### 硬盘性能测试

随机写测试: `fio -filename=./tmp_file -direct=1 -iodepth 1 -thread -rw=randwrite -ioengine=psync -bs=16k -size=1G -numjobs=10 -runtime=10 -group_reporting -name=random_write_test`

```
Jobs: 10 (f=10): [w(10)][100.0%][w=349MiB/s][w=22.3k IOPS][eta 00m:00s]
random_write_test: (groupid=0, jobs=10): err= 0: pid=31408: Sat Oct 16 15:50:10 2021
  write: IOPS=20.5k, BW=320MiB/s (335MB/s)(3196MiB/10001msec); 0 zone resets
    clat (usec): min=75, max=479413, avg=486.50, stdev=3425.03
     lat (usec): min=75, max=479414, avg=487.03, stdev=3425.03
    clat percentiles (usec):
     |  1.00th=[  172],  5.00th=[  243], 10.00th=[  277], 20.00th=[  314],
     | 30.00th=[  322], 40.00th=[  347], 50.00th=[  347], 60.00th=[  351],
     | 70.00th=[  355], 80.00th=[  367], 90.00th=[  570], 95.00th=[ 1156],
     | 99.00th=[ 2737], 99.50th=[ 3687], 99.90th=[ 6980], 99.95th=[ 9896],
     | 99.99th=[83362]
   bw (  KiB/s): min=53280, max=456960, per=98.76%, avg=323218.53, stdev=8893.42, samples=190
   iops        : min= 3330, max=28560, avg=20201.16, stdev=555.84, samples=190
  lat (usec)   : 100=0.13%, 250=5.77%, 500=82.95%, 750=2.99%, 1000=1.81%
  lat (msec)   : 2=4.41%, 4=1.55%, 10=0.34%, 20=0.03%, 50=0.01%
  lat (msec)   : 100=0.01%, 250=0.01%, 500=0.01%
  cpu          : usr=0.76%, sys=6.38%, ctx=205160, majf=0, minf=1
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,204568,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
  WRITE: bw=320MiB/s (335MB/s), 320MiB/s-320MiB/s (335MB/s-335MB/s), io=3196MiB (3352MB), run=10001-10001msec
```


顺序写测试: `fio -filename=./tmp_file -direct=1 -iodepth 1 -thread -rw=write -ioengine=psync -bs=16k -size=1G -numjobs=10 -runtime=10 -group_reporting -name=seq_write_test`

```
Jobs: 10 (f=10): [W(10)][100.0%][w=324MiB/s][w=81 IOPS][eta 00m:00s]
seq_write_test: (groupid=0, jobs=10): err= 0: pid=31308: Sat Oct 16 15:47:29 2021
  write: IOPS=111, BW=445MiB/s (467MB/s)(4536MiB/10191msec); 0 zone resets
    clat (msec): min=10, max=322, avg=88.93, stdev=27.80
     lat (msec): min=10, max=322, avg=89.08, stdev=27.80
    clat percentiles (msec):
     |  1.00th=[   22],  5.00th=[   67], 10.00th=[   75], 20.00th=[   84],
     | 30.00th=[   84], 40.00th=[   84], 50.00th=[   84], 60.00th=[   84],
     | 70.00th=[   85], 80.00th=[   90], 90.00th=[  101], 95.00th=[  153],
     | 99.00th=[  207], 99.50th=[  234], 99.90th=[  268], 99.95th=[  321],
     | 99.99th=[  321]
   bw (  KiB/s): min=262144, max=606208, per=100.00%, avg=460907.79, stdev=8941.43, samples=199
   iops        : min=   64, max=  148, avg=112.53, stdev= 2.18, samples=199
  lat (msec)   : 20=0.97%, 50=1.94%, 100=86.95%, 250=9.88%, 500=0.26%
  cpu          : usr=0.21%, sys=0.70%, ctx=2281, majf=0, minf=1
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,1134,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
  WRITE: bw=445MiB/s (467MB/s), 445MiB/s-445MiB/s (467MB/s-467MB/s), io=4536MiB (4756MB), run=10191-10191msec
```

## 折腾过程

### 单线程 1 亿条固定字符串写入测试

直接写固定长度的字符串, 没有 log 格式化过程, 测试写磁盘的极限速度

```plain
...
flush to disk, 3 buffers
flush to disk, 3 buffers
flush to disk, 9 buffers
flush to disk, 29 buffers
flush to disk, 110 buffers
flush to disk, 607 buffers
write to cache used 4129 ms, 2421.89w log per second
writeback to disk used 10663 ms, 937.82w log per second
```

log =(append)=> Buffer =(fwrite)=> File  

由于 `fwrite` 会写到系统的缓存, 实际上相当与在写内存

![page_cache](imgs/page_cache.png)  

蓝色为 Dirty Memory, 红色为 Writeback Memory

而系统缓存刷新的策略由 `/proc/sys/vm/dirty_*` 参数决定:  

`/proc/sys/vm/dirty_background_ratio` 脏页数据达到系统总内存的百分比时触发刷新, 默认`10`

`/proc/sys/vm/dirty_expire_centisecs` 脏页驻留时间超过时刷新,单位1/100s, 默认 `1500`

上面测试是内存比较充足的情况测试的, Cache 达到系统内存 10% 触发了系统的缓存刷新策略,
开始刷数据到磁盘, 双缓存瞬间被打爆, 开始疯狂 new Buffer, 大量 Buffer 开始排队等待.
上面的数据还是比较好的情况, 内存不够的时候, 性能会迅速恶化.
new Buffer 需要内存, 内存不足就去使用 Swap 分区, 需要写磁盘;
`fwrite` 写入 PageCache 又要新的内存, 内存不足脏页也要刷入磁盘,
磁盘的写入速度根本不可能遭得住, 10亿条测试写入直接把我笔记本搞卡死了...
对于这种突然的巨量日志冲击, muduo 的设计是, 只保留前 25 个 Buffer, 多余的日志直接丢弃.
之前看源码的时候还在想为什么要这么做, 现在算是了解了.

### `fwrite` 日志写入测试

原以为瓶颈在磁盘 I/O, 结果发现字符串格式化花费了大量时间.
实际上日志的产生速度还达不到上面那个疯狂的程度, 目前还没看到 2 个 Buffer 不够用的情况.

如果直接使用 `fwrite` 直接写日志, 由于存在 PageCache 缓冲, 性能也很好.
每一条日志用一次 `snprintf` 只格式化了两个参数(一个是msg字符串, 一个是序号),
平均性能如下, 用作参考值, 我们的日志尽可能达到这个速度:

```plain
write to cache used 2395 ms, 417.54w log per second
writeback to disk used 4156 ms, 240.62w log per second
```

### 单线程 1000w 条日志写入

因为要格式化时间和 msg, 这回速度慢多了.

```plain
write to cache used 5671 ms, 176.34w log per second
writeback to disk used 7614 ms, 131.34w log per second
```

使用 `perf` 工具分析发现日期格式化花费了大量时间, 确实对于年月日时分, 甚至秒这种都是可以重复利用的.

![data_format](./imgs/data_format.png)

于是使用静态变量存储复用的年月日时分时分秒, 只在秒数更新时才格式化一次, 同时用空数据在 log 中占位,
格式化日期时避免字符串拼接操作, 尽可能减少 `snprintf` 次数. 优化之后性能翻番!

单线程测试(1000w):

```plain
write to cache used 1464 ms, 341.53w log per second
writeback to disk used 2413 ms, 207.21w log per second
```

4 线程测试(4*1000w):

```plain
write to cache used 10696 ms, 373.97w log per second
writeback to disk used 13023 ms, 307.15w log per second
```

## 待改进

由于多线程写日志要加锁, 对性能来说确实有些损失, 但是没有我想象中严重, 大头时间还是在做字符串格式化去了,
格式化这块感觉已经压缩到不能再压缩了, 还要提高性能只能减少锁的影响了.

![append_mutex](./imgs/append_mutex.png)

目前性能还够用, 日志并没有成为系统的瓶颈, 如果后面有时间, 考虑为每个线程设置一个 Buffer 队列,
通过 threadlocal 缓存线程 pid(测试发现这样做的效率是直接 this_thread::get_pid() 的 115~120%),
从 map<Pid, BufferVector> 获取 Buffer. 或者固定 2 个 threadlocal Buffer 用于写日志,
只在写满了需要加入 flush 队列才加锁. 每次拿一个写满的 Buffer 与 flush 线程换一个空 Buffer.
