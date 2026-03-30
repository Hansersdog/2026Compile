# 第3章 语义分析
## [cite_start]3.3 语义分析的实践内容 [cite: 1, 4]

### [cite_start]3.3.1 实践要求 [cite: 5]
[cite_start]在本次实践内容中，我们对 C-- 语言做如下假设，可以认为这些就是 C-- 语言的特性（注意，假设 3、4、5 可能因后面的不同选做要求而有所改变）[cite: 6, 7]：
* [cite_start]**(1) 假设1**：整型 (int) 变量不能与浮点型 (float) 变量相互赋值或者相互运算 [cite: 8]。
* [cite_start]**(2) 假设2**：仅有 int 型变量才能进行逻辑运算或者作为 if 和 while 语句的条件；仅有 int 型和 float 型变量才能参与算术运算 [cite: 9, 10]。
* [cite_start]**(3) 假设3**：任何函数只进行一次定义，无法进行函数声明 [cite: 12]。
* [cite_start]**(4) 假设4**：所有变量（包括函数的形参）的作用域都是全局的，即程序中所有变量均不能重名 [cite: 11, 13]。
* [cite_start]**(5) 假设5**：结构体间的类型等价机制采用名等价 (Name Equivalence) 的方式 [cite: 14]。
* [cite_start]**(6) 假设6**：函数无法进行嵌套定义 [cite: 15]。
* [cite_start]**(7) 假设7**：结构体中的域不与变量重名，并且不同结构体中的域互不重名 [cite: 16]。

[cite_start]以上七个假设也可视为要求，违反即会导致各种语义错误，不过我们只对后面讨论的 17 种错误类型进行考察 [cite: 17][cite_start]。此外，可以安全地假设输入文件中不包含注释、八进制数、十六进制数、以及指数形式的浮点数，也不包含任何词法或语法错误（除了特别说明的针对选做要求的测试）[cite: 17]。

[cite_start]程序需要对输入文件进行语义分析（输入文件中可能包含函数、结构体、一维和高维数组）并检查如下类型的错误 [cite: 18]：
* [cite_start]**(1) 错误类型1**：变量在使用时未经定义 [cite: 19]。
* [cite_start]**(2) 错误类型2**：函数在调用时未经定义 [cite: 20]。
* [cite_start]**(3) 错误类型3**：变量出现重复定义，或变量与前面定义过的结构体名字重复 [cite: 23]。
* [cite_start]**(4) 错误类型4**：函数出现重复定义（即同样的函数名出现了不止一次定义）[cite: 24]。
* [cite_start]**(5) 错误类型5**：赋值号两边的表达式类型不匹配 [cite: 25]。
* [cite_start]**(6) 错误类型6**：赋值号左边出现一个只有右值的表达式 [cite: 26]。
* [cite_start]**(7) 错误类型7**：操作数类型不匹配或操作数类型与操作符不匹配（例如整型变量与数组变量相加减，或数组（或结构体）变量与数组（或结构体）变量相加减）[cite: 27, 28]。
* [cite_start]**(8) 错误类型8**：return 语句的返回类型与函数定义的返回类型不匹配 [cite: 29]。
* [cite_start]**(9) 错误类型9**：函数调用时实参与形参的数目或类型不匹配 [cite: 30]。
* [cite_start]**(10) 错误类型10**：对非数组型变量使用“[……]”（数组访问）操作符 [cite: 31]。
* [cite_start]**(11) 错误类型11**：对普通变量使用“(……)”或“()”（函数调用）操作符 [cite: 32]。
* [cite_start]**(12) 错误类型12**：数组访问操作符“[……]”中出现非整数（例如 a[1.5]）[cite: 33]。
* [cite_start]**(13) 错误类型13**：对非结构体型变量使用“.”操作符 [cite: 34]。
* [cite_start]**(14) 错误类型14**：访问结构体中未定义过的域 [cite: 35]。
* [cite_start]**(15) 错误类型15**：结构体中域名重复定义（指同一结构体中），或在定义时对域进行初始化（例如 struct A {int a=0;}）[cite: 36, 37]。
* [cite_start]**(16) 错误类型16**：结构体的名字与前面定义过的结构体或变量的名字重复 [cite: 38]。
* [cite_start]**(17) 错误类型17**：直接使用未定义过的结构体来定义变量 [cite: 39]。

[cite_start]**注意三点** [cite: 40]：
1.  [cite_start]关于数组类型的等价机制，同C语言一样，只要数组的基类型和维数相同我们即认为类型是匹配的，例如 int a[10][2] 和 int b[5][3] 即属于同一类型 [cite: 40]。
2.  [cite_start]允许类型等价的结构体变量之间的直接赋值，这时的语义是，对应的域相应赋值（数组域也如此，按相对地址赋值直至所有数组元素赋值完毕或目标数组域已经填满）[cite: 40]。
3.  [cite_start]对于结构体类型等价的判定，每个匿名的结构体类型我们认为均具有一个独有的隐藏名字，以此进行名等价判定 [cite: 40]。

[cite_start]**选做要求** [cite: 41]：
* [cite_start]**(1) 要求3.1**：修改假设3，使其变为“函数除了在定义之外还可以进行声明”。函数的定义仍然不可以重复出现，但函数的声明在相互一致的情况下可以重复出现 [cite: 44][cite_start]。增加错误类型18（声明但未定义）和错误类型19（多次声明互相冲突或声明与定义冲突）[cite: 45, 46, 47]。
* [cite_start]**(2) 要求3.2**：修改假设4，使其变为“变量的定义受可嵌套作用域的影响... 内层语句块中定义的变量到了外层语句块中就会消亡，不同函数体内定义的局部变量可以相互重名” [cite: 50]。
* [cite_start]**(3) 要求3.3**：修改假设5，将结构体间的类型等价机制由名等价改为结构等价 (Structural Equivalence) [cite: 51][cite_start]。注意结构等价时不要将数组展开来判断 [cite: 51]。

---

### [cite_start]3.3.2 输入格式 [cite: 52]
[cite_start]程序的输入是一个包含 C-- 源代码的文本文件，该源代码中可能会有语义错误 [cite: 53][cite_start]。程序需要能够接收一个输入文件名作为参数 [cite: 53][cite_start]。例如，假设程序名为 `cc`、输入文件名为 `test1`、程序和输入文件都位于当前目录下，那么在 Linux 命令行下运行 `./cc test1` 即可获得以 `test1` 作为输入文件的输出结果 [cite: 53, 54]。

### [cite_start]3.3.3 输出格式 [cite: 57]
[cite_start]实践内容二要求通过标准输出打印程序的运行结果 [cite: 58][cite_start]。对于没有语义错误的输入文件，程序不需要输出任何内容 [cite: 58, 59][cite_start]。对于存在语义错误的输入文件，程序应当输出相应的错误信息，其格式为 [cite: 59]：

[cite_start]`Error type [错误类型] at Line [行号]: [说明文字].` [cite: 60]

[cite_start]说明文字的内容没有具体要求，但是错误类型和出错的行号一定要正确，因为这是判断输出的错误提示信息是否正确的唯一标准 [cite: 61][cite_start]。输入文件中可能包含一个或者多个错误（但每行最多只有一个错误），程序需要将它们全部检查出来 [cite: 62]。

### [cite_start]3.3.4 验证环境 [cite: 63]
* (1) [cite_start]GNU Linux Release: Ubuntu 20.04, kernel version 5.13.0-44-generic [cite: 65]
* (2) [cite_start]GCC version 7.5.0 [cite: 66]
* (3) [cite_start]GNU Flex version 2.6.4 [cite: 67]
* (4) [cite_start]GNU Bison version 3.5.1 [cite: 68]
[cite_start]请不要在程序中使用各类方便编程的函数库（如 glib 等）[cite: 70, 71]。

---

### [cite_start]3.3.6 样例 (必做部分) [cite: 82, 83]

[cite_start]**样例 1** [cite: 84]
[cite_start]输入：[cite: 85]
```c
1 int main()
2 {
3   int i = 0;
4   j = i + 1;
5 }
```
[cite_start]输出：[cite: 91]
[cite_start]`Error type 1 at Line 4: Undefined variable "j".` [cite: 95]
[cite_start]样例输入中变量“j”未定义，因此程序可以输出如下的错误提示信息 [cite: 94]。

[cite_start]**样例 2** [cite: 96]
[cite_start]输入：[cite: 98]
```c
1 int main()
2 {
3   int i = 0;
4   inc(i);
5 }
```
[cite_start]输出：[cite: 107]
[cite_start]`Error type 2 at Line 4: Undefined function "inc".` [cite: 109]
[cite_start]样例输入中函数“inc”未定义，因此程序可以输出如下的错误提示信息 [cite: 108]。

[cite_start]**样例 3** [cite: 110]
[cite_start]输入：[cite: 111]
```c
1 int main()
2 {
3   int i, j;
4   int i;
5 }
```
[cite_start]输出：[cite: 120]
[cite_start]`Error type 3 at Line 4: Redefined variable "i".` [cite: 122]
[cite_start]样例输入中变量“i”被重复定义，因此程序可以输出如下的错误提示信息 [cite: 121]。

[cite_start]**样例 4** [cite: 123]
[cite_start]输入：[cite: 124]
```c
1 int func(int i)
2 {
3   return i;
4 }
5 
6 int func()
7 {
8   return 0;
9 }
10 
11 int main()
12 {
13 }
```
[cite_start]输出：[cite: 139]
[cite_start]`Error type 4 at Line 6: Redefined function "func".` [cite: 141]
[cite_start]样例输入中函数“func”被重复定义，因此程序可以输出如下的错误提示信息 [cite: 140]。

[cite_start]**样例 5** [cite: 142]
[cite_start]输入：[cite: 143]
```c
1 int main()
2 {
3   int i;
4   i = 3.7;
5 }
```
[cite_start]输出：[cite: 151]
[cite_start]`Error type 5 at Line 4: Type mismatched for assignment.` [cite: 155]
[cite_start]样例输入中错将一个浮点常数赋值给一个整型变量，因此程序可以输出如下的错误提示信息 [cite: 154]。

[cite_start]**样例 6** [cite: 156]
[cite_start]输入：[cite: 158]
```c
1 int main()
2 {
3   int i;
4   10 = i;
5 }
```
[cite_start]输出：[cite: 166]
[cite_start]`Error type 6 at Line 4: The left-hand side of an assignment must be a variable.` [cite: 168]
[cite_start]样例输入中整数“10”出现在了赋值号的左边，因此程序可以输出如下的错误提示信息 [cite: 167]。

[cite_start]**样例 7** [cite: 169]
[cite_start]输入：[cite: 170]
```c
1 int main()
2 {
3   float j;
4   10 + j;
5 }
```
[cite_start]输出：[cite: 177]
[cite_start]`Error type 7 at Line 4: Type mismatched for operands.` [cite: 180]
[cite_start]样例输入中表达式“10+j”的两个操作数的类型不匹配，因此程序可以输出如下的错误提示信息 [cite: 178, 179]。

[cite_start]**样例 8** [cite: 181]
[cite_start]输入：[cite: 182]
```c
1 int main()
2 {
3   float j = 1.7;
4   return j;
5 }
```
[cite_start]输出：[cite: 191]
[cite_start]`Error type 8 at Line 4: Type mismatched for return.` [cite: 194]
[cite_start]样例输入中“main”函数返回值的类型不正确，因此程序可以输出如下的错误提示信息 [cite: 193]。

[cite_start]**样例 9** [cite: 195]
[cite_start]输入：[cite: 196]
```c
1 int func(int i)
2 {
3   return i;
4 }
5 
6 int main()
7 {
8   func(1, 2);
9 }
```
[cite_start]输出：[cite: 208]
[cite_start]`Error type 9 at Line 8: Function "func(int)" is not applicable for arguments "(int, int)".` [cite: 210, 211]
[cite_start]样例输入中调用函数“func”时实参数目不正确，因此程序可以输出如下的错误提示信息 [cite: 209]。

[cite_start]**样例 10** [cite: 212]
[cite_start]输入：[cite: 213]
```c
1 int main()
2 {
3   int i;
4   i[0];
5 }
```
[cite_start]输出：[cite: 221]
[cite_start]`Error type 10 at Line 4: "i" is not an array.` [cite: 223]
[cite_start]样例输入中变量“i”不是数组型变量，因此程序可以输出如下的错误提示信息 [cite: 222]。

[cite_start]**样例 11** [cite: 224]
[cite_start]输入：[cite: 225]
```c
1 int main()
2 {
3   int i;
4   i(10);
5 }
```
[cite_start]输出：[cite: 233]
[cite_start]`Error type 11 at Line 4: "i" is not a function.` [cite: 236]
[cite_start]样例输入中变量“i”不是函数，因此程序可以输出如下的错误提示信息 [cite: 235]。

[cite_start]**样例 12** [cite: 237]
[cite_start]输入：[cite: 238]
```c
1 int main()
2 {
3   int i[10];
4   i[1.5] = 10;
5 }
```
[cite_start]输出：[cite: 247]
[cite_start]`Error type 12 at Line 4: "1.5" is not an integer.` [cite: 249]
[cite_start]样例输入中数组访问符中出现了非整型常数“1.5”，因此程序可以输出如下的错误提示信息 [cite: 248]。

[cite_start]**样例 13** [cite: 250]
[cite_start]输入：[cite: 251]
```c
1 struct Position
2 {
3   float x, y;
4 };
5 
6 int main()
7 {
8   int i;
9   i.x;
10 }
```
[cite_start]输出：[cite: 265]
[cite_start]`Error type 13 at Line 9: Illegal use of ".".` [cite: 267]
[cite_start]样例输入中变量“i”不是结构体类型变量，因此程序可以输出如下的错误提示信息 [cite: 266]。

[cite_start]**样例 14** [cite: 268]
[cite_start]输入：[cite: 269]
```c
1 struct Position
2 {
3   float x, y;
4 };
5 
6 int main()
7 {
8   struct Position p;
9   if (p.n == 3.7)
10    return 0;
11 }
```
[cite_start]输出：[cite: 287]
[cite_start]`Error type 14 at Line 9: Non-existent field "n".` [cite: 289]
[cite_start]样例输入中结构体变量“p”访问了未定义的域“n”，因此程序可以输出如下的错误信息 [cite: 288]。

[cite_start]**样例 15** [cite: 290]
[cite_start]输入：[cite: 291]
```c
1 struct Position
2 {
3   float x, y;
4   int x;
5 };
6 
7 int main()
8 {
9 }
```
[cite_start]输出：[cite: 303]
[cite_start]`Error type 15 at Line 4: Redefined field "x".` [cite: 305]
[cite_start]样例输入中结构体的域“x”被重复定义，因此程序可以输出如下的错误信息 [cite: 304]。

[cite_start]**样例 16** [cite: 306]
[cite_start]输入：[cite: 307]
```c
1 struct Position
2 {
3   float x;
4 };
5 
6 struct Position
7 {
8   int y;
9 };
10 
11 int main()
12 {
13 }
```
[cite_start]输出：[cite: 323]
[cite_start]`Error type 16 at Line 6: Duplicated name "Position".` [cite: 325]
[cite_start]样例输入中两个结构体的名字重复，因此程序可以输出如下的错误信息 [cite: 324]。

[cite_start]**样例 17** [cite: 326]
[cite_start]输入：[cite: 327]
```c
1 int main()
2 {
3   struct Position pos;
4 }
```
[cite_start]输出：[cite: 334]
[cite_start]`Error type 17 at Line 3: Undefined structure "Position".` [cite: 336]
[cite_start]样例输入中结构体“Position”未经定义，因此程序可以输出如下的错误信息 [cite: 335]。

---

### [cite_start]3.3.7 样例 (选做部分) [cite: 337, 338]

[cite_start]**样例 1** [cite: 339]
[cite_start]输入：[cite: 340]
```c
1 int func(int a);
2 
3 int func(int a)
4 {
5   return 1;
6 }
7 
8 int main()
9 {
10 }
```
[cite_start]输出：[cite: 353]
* [cite_start]如果程序需要完成要求 3.1，这个样例输入不存在任何词法、语法或语义错误，因此不需要输出错误信息 [cite: 354, 355]。
* 如果程序不需要完成要求 3.1，这个样例输入存在语法错误，因此程序可以输出如下的错误提示信息：
    [cite_start]`Error type B at Line 1: Incomplete definition of function "func".` [cite: 356, 357, 358]

[cite_start]**样例 2** [cite: 359]
[cite_start]输入：[cite: 360]
```c
1 struct Position
2 {
3   float x, y;
4 };
5 
6 int func(int a);
7 
8 int func (struct Position p);
9 
10 int main()
11 {
12 }
```
[cite_start]输出：[cite: 376]
* [cite_start]如果程序需要完成要求 3.1，这个样例输入存在两处语义错误：一是函数“func”的两次声明不一致；二是函数“func”未定义，因此程序可以输出如下的错误提示信息 [cite: 377, 378]：
    [cite_start]`Error type 19 at Line 8: Inconsistent declaration of function "func".` [cite: 379]
    [cite_start]`Error type 18 at Line 6: Undefined function "func".` [cite: 380]
* [cite_start]如果程序不需要完成要求 3.1，这个样例输入存在两处语法错误，因此程序可以输出如下的错误提示信息 [cite: 382, 383]：
    [cite_start]`Error type B at Line 6: Incomplete definition of function "func".` [cite: 384]
    [cite_start]`Error type B at Line 8: Incomplete definition of function "func".` [cite: 385]

[cite_start]**样例 3** [cite: 386]
[cite_start]输入：[cite: 387]
```c
1 int func()
2 {
3   int i = 10;
4   return i;
5 }
6 
7 int main()
8 {
9   int i;
10  i = func();
11 }
```
[cite_start]输出：[cite: 403]
* [cite_start]如果程序需要完成要求 3.2，这个样例输入不存在任何词法、语法或语义错误，因此不需要输出错误信息 [cite: 404, 405]。
* [cite_start]如果程序不需要完成要求 3.2，样例输入中的变量“i”被重复定义，因此程序可以输出如下的错误信息 [cite: 406, 407]：
    [cite_start]`Error type 3 at Line 9: Redefined variable "i".` [cite: 408]

[cite_start]**样例 4** [cite: 409]
[cite_start]输入：[cite: 410]
```c
1 int func()
2 {
3   int i = 10;
4   return i;
5 }
6 
7 int main()
8 {
9   int i;
10  int i, j;
11  i = func();
12 }
```
[cite_start]输出：[cite: 426]
* [cite_start]如果程序需要完成要求 3.2，样例输入中的变量“i”被重复定义，因此程序可以输出如下的错误提示信息 [cite: 429, 430]：
    [cite_start]`Error type 3 at Line 10: Redefined variable "i".` [cite: 431]
* [cite_start]如果程序不需要完成要求 3.2，样例输入中的变量“i”被重复定义了两次，因此程序可以输出如下的错误提示信息 [cite: 432, 433]：
    [cite_start]`Error type 3 at Line 9: Redefined variable "i".` [cite: 434]
    [cite_start]`Error type 3 at Line 10: Redefined variable "i".` [cite: 435]

[cite_start]**样例 5** [cite: 436]
[cite_start]输入：[cite: 437]
```c
1 struct Temp1
2 {
3   int i;
4   float j;
5 };
6 
7 struct Temp2
8 {
9   int x;
10  float y;
11 };
12 
13 int main()
14 {
15  struct Temp1 t1;
16  struct Temp2 t2;
17  t1 = t2;
18 }
```
[cite_start]输出：[cite: 462]
* [cite_start]如果程序需要完成要求 3.3，这个样例输入不存在任何词法、语法或语义错误，因此不需要输出错误信息 [cite: 463, 464]。
