#include <unordered_set>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>

using namespace std;

struct page_node
{
	string page_num;
	int priority;
};

void move_element(vector<page_node> &list, size_t i)
{
	for(; i; --i)
	{
		page_node tmp = list[i];
		list[i] = list[i-1];
		list[i-1] = tmp;
	}
}

void add_node(vector<page_node> &list, int priority, string &page_num)
{
	page_node new_node;
	new_node.page_num = page_num;
	new_node.priority = priority;
	list.push_back(new_node);
	move_element(list, list.size()-1);
	/*for(size_t i = list.size() - 1; i; --i)
	{
		page_node tmp = list[i];
		list[i] = list[i-1];
		list[i-1] = tmp;
	}*/
}


int main(int argc, char const *argv[])
{
	bool flag = false;

	printf("FIFO---\n");

	QQ:
	printf("size\tmiss\thit\t\tpage fault ratio\n");
	for(int frame_num = 64; frame_num <= 512; frame_num *= 2)
	{
		FILE *fp = fopen("trace.txt", "r");
		int num_of_nodes = 0;
		int hit_count = 0;
		int fault_count = 0;
		int priority = 0;
		vector<page_node> list;
		unordered_set<string> table;
		
		//printf("%d\n", frame_num);
		char cstr[20];
		char mem_addr[6];
		string mem_addr_str;

		while( !fseek(fp, 3, SEEK_CUR) && fgets(cstr, 20, fp)) // skip OP
		{
			++priority;

			strncpy(mem_addr, cstr, 5);
			mem_addr_str = mem_addr;
			
			if(table.find(mem_addr_str) == table.end())
			{
				//case in page fault, the way to add node are same
				++fault_count;
				add_node(list, priority, mem_addr_str);
				table.insert(mem_addr_str);
				if(list.size() > frame_num)
				{
					string tmp =  list[list.size()-1].page_num;
					list.pop_back();
					table.erase(tmp);
				}
			}
			else
			{
				++hit_count;
				if(flag)
				{
					//++hit_count;
					//printf("fuck\n");
					size_t j = 0;
					//only LRU need to move hit page
					while(j < list.size())
					{
						if(list[j].page_num == mem_addr_str)
						{
							break;
						}
						++j;
					}
					list[j].priority = priority;
					//printf("fuck\n");
					move_element(list, j);
				}
				//printf("the.........\n");
			}
		} 

		fclose(fp);
		double qq = double(fault_count)/double(hit_count);
		printf("%d\t%d\t%d\t%lf\n", frame_num, fault_count, hit_count, qq);
	}
	if(flag == false)
	{
		printf("LRU---\n");
		flag = true;
		goto QQ;
	}

	return 0;
}
