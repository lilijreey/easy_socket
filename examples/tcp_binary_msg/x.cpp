/**
 * @file     x.cpp
 *           j
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/16/2019 12:05:54 AM
 *
 */
#include <stdio.h>


template <class T>
struct AA {

  void ok() {
    static_cast<T*>(this)->nn();
  }

  void xx() {
    static_cast<T*>(this)->mm();
  }

};


struct BB : AA<BB>
{
  void nn() {
    printf("BB::nn\n");
  }

};

struct MM {
  void mm() {printf("MM::mm()\n");}
};

//struct CC :  MM,AA<CC>
struct CC :  AA<CC>,MM
{
  void nn() {
    printf("CC::nn\n");
  }

};

template <class sub>
struct DD : AA<sub>
{
  void mm() {printf("DD::mm()\n");}
};

struct FF : DD<FF>
{
  void nn() {
    printf("FF::nn\n");
  }

};

int main()
{
  BB b;
  b.ok();
  
  MM m;
  m.mm();

  CC c;
  c.ok();
  c.mm();

  FF f;
  f.ok();
  f.mm();

}
