#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

const int32_t ORDER = 3;
const int32_t INTERNAL_NODE_LEN = 2 * ORDER - 1;
const int32_t DATA_LEN = 2;



struct InternalNode {
    vector<int64_t> kv;
};

struct LeafNode {
    vector<int64_t> data;
};

struct Node {
    uint8_t is_leaf;
    Node* parent; // if parent is NULL this node is the ROOT node
    uint32_t row_len; // number of rows if the node is a leaf
    LeafNode* leaf_node;
    InternalNode* internal_node;
};



void printInternalNode(Node* &root) {
    auto &kv = root->internal_node->kv;
    int sz = kv.size();
    cout << &root<< " --> ";
    for(int i=1;i<sz;i+=2) {
        cout << kv[i] << ", ";
    }
    cout << "\n";
}

void printLeafNode(Node* &root) {
    cout << &root << " <--> ";
    for(auto i: root->leaf_node->data) {
        cout << i << ", ";
    }
    cout << "\n";
}




void moveRight(vector<int64_t> &kv) {
    int sz = kv.size();
    kv.resize(sz+2);
    for(int i=sz+1;i>=2;--i) {
        kv[i] = kv[i-2];
    }
}

Node* splitInternalNode(Node *head) {
    auto& kv = head->internal_node->kv;
    int kvsz = kv.size();
    int mid = kvsz / 2;
    if(mid%2 == 0) --mid;
    

    Node* left_half = new Node();
    left_half->internal_node = new InternalNode();
    left_half->is_leaf = 0;

    vector<int64_t> rightnums(kv.begin() + mid + 1, kv.end());
    vector<int64_t> leftnums(kv.begin(), kv.begin()+mid);

    left_half->internal_node->kv = leftnums;
    kv = rightnums;

    int left_half_size = left_half->internal_node->kv.size();
    for(int i=0;i<left_half_size;i+=2) {
        ((Node* )(left_half->internal_node->kv[i]))->parent = left_half;
    }
    
    return left_half;
}

void insertIntoInternal(Node* head, int64_t key, Node* leftptr, Node* rightptr, Node* &root) {
    if(head == nullptr) {
        head = new Node();
        head->internal_node = new InternalNode();
        head->parent = 0;
        head->is_leaf = 0;
        head->internal_node->kv.push_back((int64_t)rightptr);
        root = head;
    }

    auto &kv = head->internal_node->kv;
    leftptr->parent = head;
    rightptr->parent = head;

    moveRight(kv);
    int kvsz = kv.size();
    
    // Insert
    int i;
    for(i=3;i<kvsz;i+=2) {
        if(kv[i] < key) {
            swap(kv[i], kv[i-2]);
            swap(kv[i-1], kv[i-3]);
        }
        else
            break;
    }
    i -= 2;
    kv[i] = key;
    kv[i-1] = (int64_t)leftptr;

    // Check and split
    if(kvsz > INTERNAL_NODE_LEN) {
        int mid = kvsz / 2;
        if(mid%2 == 0) --mid;
        int64_t midKey = kv[mid];

        Node* left_half = splitInternalNode(head);

        insertIntoInternal(head->parent, midKey, left_half, head, root);
    }
}



Node* splitLeafNode(Node* head, int64_t value) {

    auto &data = head->leaf_node->data;
    int half_size = (DATA_LEN + 2)/2;
    
    vector<int64_t> right_data(data.begin()+half_size, data.end());
    vector<int64_t> left_data(data.begin(), data.begin()+half_size);
    data = right_data;

    Node* leaf = new Node();
    leaf->leaf_node = new LeafNode();
    leaf->is_leaf = 1;
    leaf->leaf_node->data = left_data;
    
    return leaf;
}

void insertIntoLeaf(Node* head, int value, Node* &root) {
    auto &data = head->leaf_node->data;
    
    // insert
    data.push_back(value);
    sort(data.begin(), data.end());
    int sz = data.size();

    // check and split
    if(sz > DATA_LEN) {
        int mid = (sz+1) / 2;
        int64_t midKey = data[mid-1];
        Node* left_half = splitLeafNode(head, value);
        insertIntoInternal(head->parent, midKey, left_half, head, root);
    }
}



void insert(Node* head, Node* parent, int64_t value, Node* &root) {
    if(!head->is_leaf) {
        auto& kv = head->internal_node->kv;
        int sz = kv.size();
        int i;
        for(i=1;i<sz;i+=2) {
            if(kv[i] >= value) break;
        }
        insert((Node*)kv[i-1], head, value, root);
        return;
    }

    if(head == nullptr) {
        Node* ptr = new Node();
        ptr->is_leaf = 1;
        ptr->parent = parent;
        ptr->leaf_node = new LeafNode();
        head = ptr;
    }

    insertIntoLeaf(head, value, root);
}




void printInternal(Node* head, queue<pair<int, Node*>>& Q, int dis) {
    auto& kv = head->internal_node->kv;
    string snd = "";
    for(int i=1;i<(int)kv.size();i+=2) {
        snd += "," + to_string(kv[i]);
    }
    for(int i=0;i<(int)kv.size();i+=2) {
        Q.push({dis+1, (Node*)kv[i]});
    }
    cout << snd << " ";
}

void printLeaf(Node* leaf) {
    auto& data = leaf->leaf_node->data;
    string s = "";
    for(auto i: data) {
        s += "," + to_string(i);
    }
    cout << s << " ";
}

void print(Node* head) {
    queue<pair<int, Node*>> Q;
    Q.push({0, head});
    int prev = 0;

    while(Q.size()) {
        auto [dis, cur] = Q.front();
        Q.pop();

        if(dis != prev) {
            cout << "\n";
        }
        if(cur->is_leaf) {
            cout << "::";
            printLeaf(cur);
        }
        else 
            printInternal(cur, Q, dis);
        prev = dis;
    }
    cout << "\n\n";
}



int main(int argc, char* argv[]) {
    Node* root = new Node();
    root->is_leaf = 1;
    root->parent = NULL;
    root->row_len = DATA_LEN;
    root->leaf_node = new LeafNode();

    // vector<int> nums = {5, 12, 1, 2, 18 ,21};
    vector<int> nums = {5, 21, 16, 1, 6 ,2, 7, 9, 12, 18, 0};

    for(auto i: nums) {
        insert(root, nullptr, i, root);
        print(root);
    }


    cout << "\n";

    return 0;
}