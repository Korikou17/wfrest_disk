

# Day01

1. 命名空间就是定义一块空间，其中的变量、函数等可以与空间外重复，方便协作写代码，匿名空间不能被其他文件使用

2. const关键字在编译阶段定义，宏定义在预处理阶段直接进行文本替换。const有类型，可以限制作用域，宏定义全局有效。

3. 指向常量的指针不能修改值，可以修改指针指向，const int*p.常量指针可以修改值，不能修改指针指向,int *const p
   数组指针是指向数组的指针，int (*p)[5],指针数组是存储指针的数组,int* p[5]
   函数指针是指向函数的指针，比如int (*p)(int, int)。指针函数是返回指针的函数，本质是一个函数，它的返回值是指针，int* p(int a, int b)。

4. malloc/free只分配字节，返回void*，需要强制类型转换，需要传入字节数
   new/delete不需要转换类型，会自动计算类型大小，会默认初始值为0

5. 引用是变量的别名，不新开内存，指针是一个新变量，存储变量的内存地址
   引用不能更改，指针可以修改
   引用传参传的是变量本身，形参会影响实参，也不用复制内存，效率高，指针会复制一份内存，效率低
   常引用const int &a = b;是用const修饰的引用，不能修改引用绑定的变量，防止传参后函数内部改变值，可以用来传递常量，普通引用不能绑定常量和临时值，常引用可以
   大型对象比如结构体用常引用零拷贝+只读，效率高

6. 函数重载就是运行函数重名，但是形参列表不同，实现原理是.o文件中编译的时候会根据形参列表不同给函数不同的名字
   C和C++混合编程使用extern "C"{}

7. inline是内联函数，编译器在编译的时候会将inline函数的代码直接嵌入到调用处，不再产生函数调用、跳转、栈帧开辟 / 销毁等开销
   inline函数在编译期处理，有类型检查和参数检查，有运算符优先级，有作用域，可以打断点调试，带参宏定义基本是相反的

8. x, y = 10, 26
   x, y = 26, 10
   x, y = 10, 26
   x, y = 25, 11

9. 2,5

10. 27

11. 6

12. 将 i<20改成i+20
    for(int i = 0; i + 20; i--)
        cout << "hello" << endl;

13. ```c++
    #include <iostream>
    using namespace std;
    
    void processArray(int arr[],int size){
        for(int i=0;i<size;i++){
            cout<<arr[i]<<" ";
        }
        cout<<endl;
    }
    
    void processArray(double arr[],int size){
        for(int i=0;i<size;i++){
            cout<<arr[i]<<" ";
        }
        cout<<endl;
    }
    
    int main(){
        int arr[5]={1,2,3,4,5};
        double arrdouble[3]={1.2,3.4,5.6};
        processArray(arr,5);
        processArray(arrdouble,3);
    
        return 0;
    }
    
    ```

    }

14. ```
    #include <iostream>
    using namespace std;
    #define SQUARE_MACRO(x) ((x) * (x))
    
    inline int square_inline(int x) {
          return x * x;
    }
    
    int main()
    {
        cout << SQUARE_MACRO(3) << endl;
        cout << square_inline(4) << endl;
        return 0;
    }
    
    ```

15. 不适合，递归深度在编译期未知，会造成代码膨胀

16. 

```
#include <iostream>
  using namespace std;

  void show(int a, int b = 20, int c = 30) {
      cout << a << " " << b << " " << c << endl;
  }

  int add(int a, int b = 10, int c = 20) {
      return a + b + c;
  }

  int main() {
      show(1);           
      show(1, 2);        
      show(1, 2, 3);     

      cout << add(1) << endl;        
      cout << add(1, 2) << endl;     
      cout << add(1, 2, 3) << endl;  

      return 0;
  }

```

