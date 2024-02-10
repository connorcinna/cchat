typedef struct node
{
	struct node* next;
	void* data;
} node_t;

void insert(void* data);
void delete(void* data);
void search(void* data);

