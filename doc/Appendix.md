
# C-- 语言文法与虚拟机使用技术文档

本文档详细说明了 C-- 语言的文法定义、补充说明、中间代码虚拟机 IR Simulator 的使用指南，以及实验相关环境配置和资源下载信息 。

---

## 一、 C-- 语言文法定义

### 1. 词法单元 (Tokens)

词法单元主要与程序的词法分析有关 。

* 
**INT**: 表示不带空格的数字序列 。需要使用正则表达式表示整型数，假设每个整型数不超过32 bits位 。


* 
**FLOAT**: 表示包含数字和一个小数点的实数 。小数点前后必须至少包含一个数字 。只需考虑符合C语言规范的浮点常数 。


* 
**ID**: 标识符，由52个大小写字母、10个数字和一个下划线组成 。标识符不能以数字开头 。假设每个标识符长度不超过32个字符 。


* **其它符号**:
* 
`SEMI` $\rightarrow$ `;` 


* 
`COMMA` $\rightarrow$ `,` 


* 
`ASSIGNOP` $\rightarrow$ `=` 


* `RELOP` $\rightarrow$ `>` | `<` | `>=` | `<=` | `==` | `!=` 


* 
`PLUS` $\rightarrow$ `+` 


* 
`MINUS` $\rightarrow$ `-` 


* 
`STAR` $\rightarrow$ `*` 


* 
`DIV` $\rightarrow$ `/` 


* 
`AND` $\rightarrow$ `&&` 


* 
`OR` $\rightarrow$ `||` 


* 
`DOT` $\rightarrow$ `.` 


* 
`NOT` $\rightarrow$ `!` 


* `TYPE` $\rightarrow$ `int` | `float` 


* 括号类: `LP` $\rightarrow$ `(` , `RP` $\rightarrow$ `)` , `LB` $\rightarrow$ `[` , `RB` $\rightarrow$ `]` , `LC` $\rightarrow$ `{` , `RC` $\rightarrow$ `}` 


* 关键字: `STRUCT` $\rightarrow$ `struct` , `RETURN` $\rightarrow$ `return` , `IF` $\rightarrow$ `if` , `ELSE` $\rightarrow$ `else` , `WHILE` $\rightarrow$ `while` 





### 2. 高层定义 (High-level Definitions)

包含了 C-- 语言中所有的全局变量以及函数定义的语法 。

* 
`Program` $\rightarrow$ `ExtDefList` 


* `ExtDefList` $\rightarrow$ `ExtDef` `ExtDefList` | $\epsilon$ 


* `ExtDef` $\rightarrow$ `Specifier` `ExtDecList` `SEMI` | `Specifier` `SEMI` | `Specifier` `FunDec` `CompSt` 


* `ExtDecList` $\rightarrow$ `VarDec` | `VarDec` `COMMA` `ExtDecList` 



### 3. 类型描述符 (Specifiers)

主要与变量的类型有关 。

* `Specifier` $\rightarrow$ `TYPE` | `StructSpecifier` 


* `StructSpecifier` $\rightarrow$ `STRUCT` `OptTag` `LC` `DefList` `RC` | `STRUCT` `Tag` 


* `OptTag` $\rightarrow$ `ID` | $\epsilon$ 


* 
`Tag` $\rightarrow$ `ID` 



### 4. 声明 (Declarators)

主要与变量和函数的定义有关 。

* `VarDec` $\rightarrow$ `ID` | `VarDec` `LB` `INT` `RB` 


* `FunDec` $\rightarrow$ `ID` `LP` `VarList` `RP` | `ID` `LP` `RP` 


* `VarList` $\rightarrow$ `ParamDec` | `ParamDec` `COMMA` `VarList` 


* 
`ParamDec` $\rightarrow$ `Specifier` `VarDec` 



### 5. 语句 (Statements)

主要与语句相关 。

* 
`CompSt` $\rightarrow$ `LC` `DefList` `StmtList` `RC` 


* `StmtList` $\rightarrow$ `Stmt` `StmtList` | $\epsilon$ 


* `Stmt` $\rightarrow$ `Exp` `SEMI` | `CompSt` | `RETURN` `Exp` `SEMI` 


* `Stmt` $\rightarrow$ `IF` `LP` `Exp` `RP` `Stmt` | `IF` `LP` `Exp` `RP` `Stmt` `ELSE` `Stmt` | `WHILE` `LP` `Exp` `RP` `Stmt` 



### 6. 局部定义 (Local Definitions)

主要与局部变量的定义有关 。

* `DefList` $\rightarrow$ `Def` `DefList` | $\epsilon$ 


* 
`Def` $\rightarrow$ `Specifier` `DecList` `SEMI` 


* `DecList` $\rightarrow$ `Dec` | `Dec` `COMMA` `DecList` 


* `Dec` $\rightarrow$ `VarDec` | `VarDec` `ASSIGNOP` `Exp` 



### 7. 表达式 (Expressions)

主要与表达式有关 。

* 表达式类型：
* 
**二元运算**: `Exp` `ASSIGNOP` `Exp`, `Exp` `AND` `Exp`, `Exp` `OR` `Exp`, `Exp` `RELOP` `Exp`, `Exp` `PLUS` `Exp`, `Exp` `MINUS` `Exp`, `Exp` `STAR` `Exp`, `Exp` `DIV` `Exp` 。


* 
**一元与特殊**: `LP` `Exp` `RP`, `MINUS` `Exp`, `NOT` `Exp`, `ID` `LP` `Args` `RP`, `ID` `LP` `RP`, `Exp` `LB` `Exp` `RB`, `Exp` `DOT` `ID` 。


* 
**基本要素**: `ID`, `INT`, `FLOAT` 。




* `Args` $\rightarrow$ `Exp` `COMMA` `Args` | `Exp` 



---

## 二、 文法补充说明

### 1. 词法细节

* 
**整型数 (INT)**: 表示所有（无符号）整型常数 。十进制整数由0~9数字组成，中间无空格分隔符，除“0”之外首位数字不为0（例如：0、234、10000） 。可假设输入均在32bits位之内 。还支持八进制（由0~7组成，0开头，如0237表示十进制159）和十六进制（由0~9、A~F或a~f组成，以0x或0X开头，如0xFF32表示十进制65330） 。


* 
**浮点数 (FLOAT)**: 表示所有（无符号）浮点型常数 。由一串数字与小数点组成，小数点前后必须有数字（如0.7、12.43） 。符合IEEE754单精度标准（对应C语言中的float类型） 。支持指数形式（科学记数法）：包括基数、指数符号（E或e）和指数三部分，依次出现 。基数部分小数点可在数字串任何位置，指数部分可带“+”或“-”（如01.23E12、43.e-4、5E03） 。


* 
**标识符 (ID)**: 除去保留字外的所有标识符，由大小写字母、数字和下划线组成，必须以字母或下划线开头 。假设长度小于32个字符 。


* 
**其它词法单元**: 产生式中箭头右边的具体字符串会被识别为对应的词法单元（例如 `int` 被识别为 `TYPE`） 。



### 2. 语法细节

* 
**Program**: 初始语法单元，表示整个程序，可产生零个或多个 `ExtDef` 组成的 `ExtDefList` 。


* **ExtDef (全局定义)**:
* 
`ExtDef` $\rightarrow$ `Specifier ExtDecList SEMI`: 表示全局变量的定义 。


* 
`ExtDef` $\rightarrow$ `Specifier SEMI`: 专门为结构体定义准备（如 `struct {...};`），允许类似 `int;` 这种无意义但符合标准的合法语句（不需要报错） 。


* 
`ExtDef` $\rightarrow$ `Specifier FunDec CompSt`: 表示函数的定义 。




* **StructSpecifier (结构体定义)**:
* 基本格式：`STRUCT OptTag LC DefList RC` 。其中 `OptTag` 可有可无 。


* 使用已定义结构体：`STRUCT Tag`（如 `struct Complex a, b;`） 。




* 
**VarDec 与 FunDec**: `VarDec` 可以是简单标识符或数组变量（后面跟方括号括起来的数字） 。`FunDec` 表示函数头，包含函数名和形参列表 `VarList` 。


* 
**CompSt (语句块)**: 由花括号括起来的语句块，内部先是一系列的变量定义 `DefList`，然后是一系列的语句 `StmtList` 。**必须在每一个语句块的开头才可以定义变量**，不允许在任意位置定义 。


* 
**Local Definitions (局部定义)**: `DefList` 可由零个或多个 `Def` 组成 。允许在定义局部变量时进行初始化（如 `int a = 5;`） 。


* **Comments (注释)**:
* 单行注释：使用双斜线 `//`，直接丢弃该行其后的所有字符 。


* 多行注释：使用 `/*` 和 `*/` 包裹 。**注意：`/*` 与 `*/` 不允许嵌套**，否则编译器需报错 。





### 3. 运算符优先级和结合性

为了消除潜在的二义性问题，规定了各运算符的优先级和结合性（数值越小表示代表优先级越高 ）：

| 优先级 | 运算符 | 结合性 | 描述 |
| --- | --- | --- | --- |
| 1 | `( )` | / | 括号或函数调用 

 |
| 1 | `[ ]` | 左结合 | 数组访问 

 |
| 1 | `.` | 左结合 | 结构体访问 

 |
| 2 | `-` | 右结合 | 取负 

 |
| 2 | `!` | 右结合 | 逻辑非 

 |
| 3 | `*` | 左结合 | 乘 

 |
| 3 | `/` | 左结合 | 除 

 |
| 4 | `+` | 左结合 | 加 

 |
| 4 | `-` | 左结合 | 减 

 |
| 5 | `<` | 左结合 | 小于 

 |
| 5 | `<=` | 左结合 | 小于或等于 

 |
| 5 | `>` | 左结合 | 大于 

 |
| 5 | `>=` | 左结合 | 大于或等于 

 |
| 6 | `==` | 左结合 | 等于 

 |
| 6 | `!=` | 左结合 | 不等于 

 |
| 7 | `&&` | 左结合 | 逻辑与 

 |
| 8 | ` |  | ` |
| 9（注：原表描述为8） | `=` | 右结合 | 赋值 

 |

---

## 三、 IR Simulator 虚拟机使用说明

IR Simulator 是一个本质上为中间代码解释器的虚拟机小程序，由 Python 写成，借助 Qt 库实现图形界面，仅在 Linux 下发布 。

### 1. 界面与基本操作

* 
**界面划分**: 分为左侧的代码区（显示载入的中间代码）、右上方的监视区（显示当前执行函数的参数与局部变量的值）和右下方的控制台区（供 WRITE 函数进行输出用） 。


* 
**载入代码**: 点击工具栏第一个按钮或通过菜单 `File → Open` 打开以 `.ir` 为后缀名的文本文件 。若包含语法错误，程序会弹出对话框提示并拒绝载入 。


* **执行操作**:
* 运行：点击第二个按钮或 `Run → Run`（快捷键 F5） 。


* 单步执行：点击第三个按钮或 `Run → Step`（快捷键 F8） 。


* 停止：点击第四个按钮或 `Run → Stop`，停止并重新初始化 。




* 
**运行结果与错误处理**: 若运行出错，会弹出对话框提示出错行号及原因（如访问不存在地址，或缺少 RETURN 等） 。运行正常结束后，对话框会提示 `Program has exited gracefully.` 并显示总共执行的指令数量 (`Total instructions`) 。指令数量越小，代表优化效果越好 。



### 2. 核心注意事项

* 
**死循环问题**: 运行前需确保代码不会陷入死循环或无穷递归，否则需要强制退出程序，IR Simulator 不会对此进行判断 。


* 
**参数匹配**: 互相对应的 `ARG` 语句和 `PARAM` 语句数量一定要相等，否则会导致未知的错误 。


* 
**跳转指令单步调试**: 单步执行时，程序不会在 `GOTO`、`IF`、`RETURN` 等跳转相关语句上停留，而是直接将控制转移到跳转目标 。


* **变量监控机制**:
* 监视区始终仅显示当前正在执行函数的变量及其值，函数刚运行时默认初始值为0 。


* 使用 `DEC` 语句定义的变量，其内容会被一对中括号 `[` 和 `]` 包裹，若内容较长可调整列宽 。


* 所有参数和局部变量在运行到该函数后才会为其分配空间，采用由低地址到高地址的栈式分配方式 。递归调用的局部变量不影响上层函数的值。想要修改上层函数变量的值需进行引用调用（传递地址参数） 。监视区只能看到当前层级的变量值 。





