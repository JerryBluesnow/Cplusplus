#include <list>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/lambda/lambda.hpp>
#include <iostream>
using namespace std;
/*
标题:boost.Multi_Index库的使用
功能：类似std::map的容器，但是可以用多个关键词索引数据
描述：boost::multi_index::multi_index_container为容器内的数据建立多个视图
     不同的索引字段(Key)对应不同的视图
	 boost::multi_index::multi_index_container经常可以采用不同的表达方式实现
	 同样的功能。
	 这里只列举一种能满足我们添加、删除、修改、检索记录的需求示例代码，有兴趣的话
	 可以通过参考资料中的官网地址得到如何使用其它方式达到这里功能需求的知识。
环境：Windows 8.1 64bit（英文版）
	  Visual Studio 2013 Professional with SP1
	  boost 1.55
注意：编写和template相关的代码一旦出错不会给出正确的错误位置
      所以建议每完成一个函数，就写一个测试这个函数的函数。
最后更新日期：2014-5-10
应用范围：定制自己的内存数据库
参考资料：http://www.boost.org/doc/libs/1_55_0/libs/multi_index/doc/index.html
*/

#pragma region 定义 employee 
//使用名字空间是为了防止数据类型名称冲突
namespace kagula
{
	namespace datatype
	{
		//为了后面的代码能通过id和name名字取视图，定义了这两个结构
		struct id{};
		struct name{};
    
		//employee是我们示例代码中表的结构
		struct employee
		{
			int          id;
			std::string name;

			employee(int id, const std::string& name) :id(id), name(name){}

			//重载<运算符函数让后面名为id的视图按照id字段的升序排列
			bool operator<(const employee& e)const{ return id<e.id; }
			//重载<=运算符函数是为了得到指定id字段值范围的记录
			bool operator<=(const employee& e)const{ return id<=e.id; }
			//重载<<运算符函数是为了我们方便打印记录的内容
			friend std::ostream& operator<<(std::ostream& os, const employee& dt);
		};
		std::ostream& operator<<(std::ostream& os, const employee& dt)
		{
			os << "[" << dt.id << ',' << dt.name.c_str() << "]";
			return os;
		}
		//后面的代码，更新指定记录name字段的值要用到change_name类
		//通过这个示例你可以知道如何修改符合搜索条件的，指定记录的任意字段值
		struct change_name
		{
			change_name(const std::string& new_name) :new_name(new_name){}

			void operator()(employee& e)
			{
				e.name = new_name;
			}

		private:
			std::string new_name;
		};

		typedef boost::multi_index::multi_index_container<
			employee,

			boost::multi_index::indexed_by<

			boost::multi_index::ordered_unique<boost::multi_index::tag<id>, boost::multi_index::identity<employee> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<name>, boost::multi_index::member<employee, std::string, &employee::name> >
			>

		> employee_set;
	}
}
#pragma endregion

using namespace kagula::datatype;


void print_out_by_id(const employee_set& es)
{
	const employee_set::index<id>::type& id_index = es.get<id>();
	
	std::copy(
		id_index.begin(), id_index.end(),
		std::ostream_iterator<employee, char>(std::cout));
	std::cout << std::endl << std::endl;
}

void print_out_by_name(const employee_set& es)
{
	const employee_set::index<name>::type& name_index = es.get<name>();

	std::copy(
		name_index.begin(), name_index.end(),
		std::ostream_iterator<employee,char>(std::cout));
	std::cout << std::endl << std::endl;
}

//建立测试数据
void CreateSample(employee_set& table)
{
	table.insert(employee(0, "Z"));
	table.insert(employee(1, "Z"));
	table.insert(employee(2, "X"));
	table.insert(employee(3, "Z"));


	table.insert(employee(100, "Judy Smith"));
	table.insert(employee(101, "Judy Smith"));

	table.insert(employee(200, "Anna Jones"));
	table.insert(employee(201, "Anna Jones"));
}


void select_stat(employee_set& table)
{
	print_out_by_id(table);

	{
		const employee_set::index<id>::type&  viewId = table.get<id>();
		std::copy(viewId.rbegin(), viewId.rend(),
			std::ostream_iterator<employee, char>(std::cout));
		std::cout << std::endl << std::endl;
	}

	print_out_by_name(table);

	{
		employee cond(2, "");
		employee_set::index<id>::type::iterator iter = table.find(cond);
		std::cout << *iter << std::endl << std::endl;
	}

	{
		std::pair<employee_set::iterator, employee_set::iterator> p;
		p = table.range(employee(100, "") <= boost::lambda::_1, boost::lambda::_1 <= employee(200, "")); // 100<= x <=200
		for (employee_set::iterator it = p.first; it != p.second; it++)
		{
			std::cout << *it << " , ";
		}
		std::cout << std::endl << std::endl;
	}

  {
		std::pair<employee_set::iterator, employee_set::iterator> p;
		p = table.range(employee(100, "") <= boost::lambda::_1, boost::lambda::_1 <= employee(200, "")); // 100<= x <=200
		for (employee_set::iterator it = p.first; it != p.second; it++)
		{
			std::cout << *it << " , ";
		}
		std::cout << std::endl << std::endl;
	}

	{
		const employee_set::index<name>::type& viewName = table.get<name>();
		employee_set::index<name>::type::iterator it = viewName.find("Judy Smith");
		std::cout << *it << std::endl << std::endl;

		unsigned int count = viewName.count("Judy Smith");
		std::cout << count << std::endl << std::endl;
	}

	{
		const employee_set::index<name>::type& viewName = table.get<name>();
		std::pair<employee_set::index<name>::type::iterator, employee_set::index<name>::type::iterator> p;

		p = viewName.equal_range("Z"); 

		for (employee_set::index<name>::type::iterator it = p.first; it != p.second; it++)
		{
			std::cout << *it << " , ";
		}
		std::cout << std::endl << std::endl;
	}

}

void delete_stat(employee_set& table)
{
	{
		employee cond(2, "");
		int nCount = table.erase(cond); 
		std::cout << "has " << nCount << " be deleted" << std::endl << std::endl;
	}

	{
		employee_set::index<name>::type& viewName = table.get<name>();
		while ( table.erase(*viewName.find("Z")) > 0 );
		print_out_by_id(table);
	}

	table.clear();
	CreateSample(table);

	std::pair<employee_set::iterator, employee_set::iterator> p;
	p = table.range(employee(100, "") <= boost::lambda::_1, boost::lambda::_1 <= employee(200, "")); // 100<= x <=200
	table.erase(p.first, p.second);
	print_out_by_id(table);
}

void update_stat(employee_set& table)
{
	typedef employee_set::index<name>::type employee_set_by_name;
	employee_set_by_name& name_index = table.get<name>();

	employee_set_by_name::iterator it = name_index.find("Anna Jones");
	name_index.modify_key(it, boost::lambda::_1 = "Anna Smith");
	print_out_by_id(table);

	it = name_index.find("Anna Smith");
	name_index.modify(it, change_name("Anna Jones"));
	print_out_by_id(table);


	{
		employee_set_by_name& viewName = table.get<name>();
		employee_set_by_name::iterator it;
		while ( (it = viewName.find("Z")) != viewName.end() )
		{
			viewName.modify(it, change_name("kagula"));
		}
		print_out_by_id(table);

		viewName = table.get<name>();
		std::pair<employee_set_by_name::iterator, employee_set_by_name::iterator> p;
		p = viewName.equal_range("kagula");
		for (employee_set_by_name::iterator it = p.first; it != p.second; it++)
		{
			viewName.modify(it, change_name("Z"));
		}
		print_out_by_id(table);
	}
}

int main(int argc, _TCHAR* argv[])
{
	//等价SQL,Add语句的相关功能
	employee_set table;
	CreateSample(table);

	//这里测试等价SQL,Select语句的相关功能
	select_stat(table);
	
	//这里测试等价SQL,Update语句的相关功能
	update_stat(table);

	typedef employee_set::index<name>::type employee_set_by_name;
	employee_set_by_name& name_index = table.get<name>();

	std::pair<employee_set_by_name::iterator, employee_set_by_name::iterator> pp;
	pp = name_index.range("Z" <= boost::lambda::_1, boost::lambda::_1 <= "Z");
	for_each(pp.first, pp.second, std::cout << boost::lambda::_1 << ' ');

	//这里测试等价SQL,Delete语句的相关功能
	delete_stat(table);



	//暂停，输入任意键继续
	system("pause");
	return 0;
}