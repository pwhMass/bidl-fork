# Byzantine Fault-Tolerant (BFT) State Machine Replication (SMaRt) v1.2
# 拜占庭容错(BFT)状态机复制(SMaRt) v1.2

This is a Byzantine fault-tolerant state machine replication project named BFT-SMaRt, a Java open source library maintained by the LaSIGE research unit at the University of Lisbon.

这是一个名为BFT-SMaRt的拜占庭容错状态机复制项目，是由里斯本大学LaSIGE研究单位维护的Java开源库。

This package contains the source code (src/), jar file (bin/BFT-SMaRt.jar), dependencies (lib/), documentation (doc/), running scripts (runscripts/), and configuration files (config/) for version 1.2 of the project.
BFT-SMaRt requires the Java Runtime Environment version 1.8 or later.

该软件包包含了项目1.2版本的源代码(src/)、jar文件(bin/BFT-SMaRt.jar)、依赖项(lib/)、文档(doc/)、运行脚本(runscripts/)和配置文件(config/)。
BFT-SMaRt需要Java运行环境1.8或更高版本。

## Quick start
## 快速开始

To run any demonstration you first need to configure BFT-SMaRt to define the protocol behavior and the location of each replica.

要运行任何演示，你首先需要配置BFT-SMaRt以定义协议行为和每个副本的位置。

The servers must be specified in the configuration file (see `config/hosts.config`):

服务器必须在配置文件中指定(参见`config/hosts.config`):

```
#server id, address and port (the ids from 0 to n-1 are the service replicas) 
#服务器id、地址和端口(从0到n-1的id是服务副本)
0 127.0.0.1 11000 11001
1 127.0.0.1 11010 11011
2 127.0.0.1 11020 11021
3 127.0.0.1 11030 11031
```

**Important tip #1:** Always provide IP addresses instead of hostnames. If a machine running a replica is not correctly configured, BFT-SMaRt may fail to bind to the appropriate IP address and use the loopback address instead (127.0.0.1). This phenomenom may prevent clients and/or replicas from successfully establishing a connection among them.

**重要提示#1:** 始终提供IP地址而不是主机名。如果运行副本的机器未正确配置，BFT-SMaRt可能无法绑定到适当的IP地址，而是使用回环地址(127.0.0.1)。这种现象可能会阻止客户端和/或副本与它们之间建立连接。

**Important tip #2:** Clients requests should not be issued before all replicas have been properly initialized. Replicas are ready to process client requests when each one outputs `-- Ready to process operations` in the console.

**重要提示#2:** 客户端请求不应在所有副本都正确初始化之前发出。副本准备好处理客户端请求时，每个副本都会输出`-- Ready to process operations`在控制台中。

The system configurations also have to be specified (see`config/system.config`). Most of the parameters are self explanatory.

系统配置也必须指定(见`config/system.config`)。大多数参数都是不言自明的。

**Important tip #3:** When using the library in real systems, always make sure to set `system.communication.defaultkeys` to `false` and `system.communication.useSignatures` to `1`. Also make sure that only the `config/keys` directory only has the private key for the repective replica/client.

**重要提示#3:** 在实际系统中使用库时，请确保将`system.communication.defaultkeys`设置为`false`，并将`system.communication.useSignatures`设置为`1`。还要确保`config/keys`目录中只有相应副本/客户端的私钥。

You can run the counter demonstration by executing the following commands, from within the main directory across four different consoles (4 replicas, to tolerate 1 fault):

你可以通过执行以下命令，从四个不同的控制台(4个副本，以容忍1个故障)，运行计数器演示，如下所示:

```
./runscripts/smartrun.sh bftsmart.demo.counter.CounterServer 0
./runscripts/smartrun.sh bftsmart.demo.counter.CounterServer 1
./runscripts/smartrun.sh bftsmart.demo.counter.CounterServer 2
./runscripts/smartrun.sh bftsmart.demo.counter.CounterServer 3
```

**Important tip #4:** If you are getting timeout messages, it is possible that the application you are running takes too long to process the requests or the network delay is too high and PROPOSE messages from the leader does not arrive in time, so replicas may start the leader change protocol. To prevent that, try to increase the `system.totalordermulticast.timeout` parameter in 'config/system.config'.

**重要提示#4:** 如果你收到超时消息，可能是因为你运行的应用程序处理请求的时间过长，或者网络延迟过高，导致领导者发送的PROPOSE消息无法及时到达，因此副本可能会开始领导变更协议。为了防止这种情况，请尝试增加`config/system.config`中的`system.totalordermulticast.timeout`参数。

**Important tip #5:** Never forget to delete the `config/currentView` file after you modify `config/hosts.config` or `config/system.config`. If `config/currentView` exists, BFT-SMaRt always fetches the group configuration from this file first. Otherwise, BFT-SMaRt fetches information from the other files and creates `config/currentView` from scratch. Note that `config/currentView` only stores information related to the group of replicas. You do not need to delete this file if, for instance, you want to enable the debugger or change the value of the request timeout.

**重要提示#5:** 永远不要忘记在修改`config/hosts.config`或`config/system.config`后删除`config/currentView`文件。如果`config/currentView`存在，BFT-SMaRt总是从该文件获取组配置。否则，BFT-SMaRt从其他文件获取信息并从头开始创建`config/currentView`。请注意，`config/currentView`仅存储与副本组相关的信息。如果你只想启用调试器或更改请求超时值，则不需要删除此文件。

Once all replicas are ready, the client can be launched as follows:

一旦所有副本都准备就绪，客户端可以如下启动:

```
./runscripts/smartrun.sh bftsmart.demo.counter.CounterClient 1001 <increment> [<number of operations>]
```

If `<increment>` equals 0 the request will be read-only. Default `<number of operations>` equals 1000.

如果`<increment>`等于0，请求将是只读的。默认的`<number of operations>`等于1000。

**Important tip #6:** always make sure that each client uses a unique ID. Otherwise, clients may not be able to complete their operations.

**重要提示#6:** 请确保每个客户端使用唯一的ID。否则，客户端可能无法完成其操作。
  
## State transfer protocol(s)
## 状态传输协议

BFT-SMaRt offers two state transfer protocols. The first is a basic protocol that can be used by extending the classes `bftsmart.tom.server.defaultservices.DefaultRecoverable` and `bftsmart.tom.server.defaultservices.DefaultSingleRecoverable`. Thee classes logs requests into memory and periodically takes snapshots of the application state.

BFT-SMaRt提供两种状态转移协议。第一种是基本协议，可以通过扩展类`bftsmart.tom.server.defaultservices.DefaultRecoverable`和`bftsmart.tom.server.defaultservices.DefaultSingleRecoverable`来使用。这些类将请求记录到内存中，并定期对应用程序状态进行快照。

The second, more advanced protocol can be used by extending the class 
`bftsmart.tom.server.defaultservices.durability.DurabilityCoordinator`. This protocol stores its logs to disk. To mitigate the latency of writing to disk, such tasks is done in batches and in parallel with the requests' execution. Additionally, the snapshots are taken at different points of the execution in different replicas.

第二种，更高级的协议可以通过扩展类`bftsmart.tom.server.defaultservices.durability.DurabilityCoordinator`来使用。此协议将其日志存储到磁盘上。为了减轻写入磁盘的延迟，这些任务是在批量处理和并行处理请求执行时完成的。此外，快照是在执行的不同点在不同副本中完成的。

**Important tip #7:** We recommend developers to use `bftsmart.tom.server.defaultservices.DefaultRecoverable`, since it is the most stable of the three classes.

**重要提示#7:** 我们建议开发人员使用`bftsmart.tom.server.defaultservices.DefaultRecoverable`，因为它是最稳定的三个类之一。

**Important tip #8:** regardless of the chosen protocol, developers must avoid using Java API objects like `HashSet` or `HashMap`, and use `TreeSet` or `TreeMap` instead. This is because serialization of Hash* objects is not deterministic, i.e, it generates different byte arrays for equal objects. This will lead to problems after more than `f` replicas used the state transfer protocol to recover from failures.

**重要提示#8:** 无论选择哪种协议，开发人员都必须避免使用Java API对象，如`HashSet`或`HashMap`，而是使用`TreeSet`或`TreeMap`。这是因为序列化Hash*对象不是确定性的，即它为相等的对象生成不同的字节数组。这将在超过`f`个副本使用状态转移协议从故障中恢复后导致问题。

## Group reconfiguration
## 组重配置

The library also implements a reconfiguration protocol that can be used to add/remove replicas from the initial group. You can add/remove replicas on-the-fly by executing the following commands:

该库还实现了重新配置协议，可以用于向初始组添加/删除副本。你可以通过执行以下命令添加/删除副本:

```
./runscripts/smartrun.sh bftsmart.reconfiguration.util.DefaultVMServices <smart id> <ip address> <port> (to add a replica to the group)
./runscripts/smartrun.sh bftsmart.reconfiguration.util.DefaultVMServices <smart id> (to remove a replica from the group)
```

**Important tip #9:** everytime you use the reconfiguration protocol, you must make sure that all replicas and the host where you invoke the above commands have the latest `config/currentView` file. The current implementation of BFT-SMaRt does not provide any mechanism to distribute this file, so you will need to distribute it on your own (e.g., using the `scp` command). You also need to make sure that any client that starts executing can read from the latest `config/currentView` file.

**重要提示#9:** 每次使用重新配置协议时，必须确保所有副本和你在上面命令中调用的主机具有最新的`config/currentView`文件。当前的BFT-SMaRt实现不提供任何机制来分发此文件，因此你需要自己分发它(例如，使用`scp`命令)。你还必须确保任何开始执行的客户端可以从最新的`config/currentView`文件中读取。

## BFT-SMaRt under crash faults
## 崩溃故障模式下的BFT-SMaRt

You can run BFT-SMaRt in crash-faults only mode by setting the `system.bft` parameter in the configuration file to `false`. This mode requires less replicas to execute, but will not withstand full Byzantine behavior from compromised replicas.

你可以通过在配置文件中将`system.bft`参数设置为`false`来运行BFT-SMaRt仅在崩溃故障模式下。此模式需要较少的副本执行，但无法承受来自被破坏副本的完整拜占庭行为。

## Generating public/private key pairs
## 生成公钥/私钥对

If you need to generate public/private keys for more replicas or clients, you can use the following command:

如果你需要为更多副本或客户端生成公共/私有密钥，可以使用以下命令:

```
./runscripts/smartrun.sh bftsmart.tom.util.RSAKeyPairGenerator <id> <key size>
```

Keys are stored in the `config/keys` folder. The command above creates key pairs both for clients and replicas. Alternatively, you can set the `system.communication.defaultkeys` to `true` in the `config/system.config` file to forces all processes to use the same public/private keys pair and secret key. This is useful when deploying experiments and benchmarks, because it enables the programmer to avoid generating keys for all principals involved in the system. However, this must not be used in a real deployments.

密钥存储在`config/keys`文件夹中。上述命令为客户端和副本创建密钥对。或者，你可以在`config/system.config`文件中将`system.communication.defaultkeys`设置为`true`，以强制所有进程使用相同的公共/私有密钥对和秘密密钥。这对于部署实验和基准测试非常有用，因为它使程序员能够避免为系统中的所有主要参与者生成密钥。但是，这不能用于实际部署。

## Compiling
## 编译

Make sure that you have Ant installed and simply type `ant` in the main directory. The jar file is stored in the `bin/` directory.

确保你已经安装了Ant，然后在主目录中简单地键入`ant`。jar文件存储在`bin/`目录中。

## Additional information and publications
## 其他信息和出版物

If you are interested in learning more about BFT-SMaRt, you can read:

如果你对学习更多关于BFT-SMaRt的信息感兴趣，你可以阅读:

- The paper about its state machine protocol published in [EDCC'12](http://www.di.fc.ul.pt/~bessani/publications/edcc12-modsmart.pdf)
- The paper about its advanced state transfer protocol published in [Usenix'13](http://www.di.fc.ul.pt/~bessani/publications/usenix13-dsmr.pdf)
- The tool description published in [DSN'14](http://www.di.fc.ul.pt/~bessani/publications/dsn14-bftsmart.pdf)

- 关于其状态机协议的论文，发表在[EDCC'12](http://www.di.fc.ul.pt/~bessani/publications/edcc12-modsmart.pdf)
- 关于其高级状态转移协议的论文，发表在[Usenix'13](http://www.di.fc.ul.pt/~bessani/publications/usenix13-dsmr.pdf)
- 关于工具描述的论文，发表在[DSN'14](http://www.di.fc.ul.pt/~bessani/publications/dsn14-bftsmart.pdf)

***Feel free to contact us if you have any questions!***
***如果你有任何问题，请随时与我们联系！***
