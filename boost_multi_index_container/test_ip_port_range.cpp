#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "range_index.h"

using namespace std;

void
test_range_index_compare()
{
    while (1)
    {
        char operation;
        cout << "press \'b\' to  break, or any other keys to continue...." << endl;
        operation = cin.get();
        if (operation == 'b')
        {
            break;
        }
        cout << endl;

        range_index first;
        range_index second;
        cout << "please input the first range index_start:";
        cin >> first.index_start;
        cout << "please input the first range index_end:";
        cin >> first.index_end;
        cout << "please input the second range index_start:";
        cin >> second.index_start;
        cout << "please input the second range index_end:";
        cin >> second.index_end;
        cout << endl;

        cout << "(" << first.index_start << "," << first.index_end << ") "
             << "(" << second.index_start << "," << second.index_end << ")"
             << ", with < : " << (first < second) << ", with > : " << (first > second)
             << ", with == : " << (first == second) << endl;

    }
    cout << "stop testing successfully........" << endl;
    return;
}

void
test_range_index_compare2(unsigned int x, unsigned int y, unsigned int xx, unsigned int yy)
{
    range_index first(x, y);
    range_index second(xx, yy);

    cout << "(" << first.index_start << "," << first.index_end << ") "
         << "(" << second.index_start << "," << second.index_end << ")"
         << ", with < : " << (first < second) << ", with > : " << (first > second)
         << ", with == : " << (first == second) << endl;

    return;
}

int main() {

  //test_range_index_compare();
  test_range_index_compare2(1, 1, 4, 10);
  test_range_index_compare2(1, 2, 4, 10);
  test_range_index_compare2(1, 4, 4, 10);
  test_range_index_compare2(1, 6, 4, 10);
  test_range_index_compare2(1, 10, 4, 10);
  test_range_index_compare2(1, 11, 4, 10);
  test_range_index_compare2(4, 4, 4, 10);
  test_range_index_compare2(4, 6, 4, 10);
  test_range_index_compare2(4, 10, 4, 10);
  test_range_index_compare2(4, 11, 4, 10);
  test_range_index_compare2(5, 5, 4, 10);
  test_range_index_compare2(5, 6, 4, 10);
  test_range_index_compare2(5, 10, 4, 10);
  test_range_index_compare2(5, 11, 4, 10);
  test_range_index_compare2(10, 10, 4, 10);
  test_range_index_compare2(10, 11, 4, 10);
  test_range_index_compare2(11, 11, 4, 10);
  test_range_index_compare2(11, 12, 4, 10);

  test_range_index_compare2(1, 1, 4, 4);
  test_range_index_compare2(11, 11, 11, 11);
  test_range_index_compare2(4, 4, 1, 1);
  test_range_index_compare2(1, 2, 4, 4);
  test_range_index_compare2(1, 4, 4, 4);
  test_range_index_compare2(1, 6, 4, 4);
  test_range_index_compare2(4, 6, 4, 4);
  test_range_index_compare2(6, 6, 4, 4);
  test_range_index_compare2(6, 10, 4, 4);

  system("pause");
  return 0;
}