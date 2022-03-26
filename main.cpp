#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>

using namespace std;

const int32_t ORDER = 7;
const int32_t INTERNAL_NODE_LEN = 2 * ORDER - 1;
const int32_t DATA_LEN = ORDER - 1;
const int32_t MIN_DATA_LEN = DATA_LEN/2;



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

// Search

Node* findBlock(Node* head, int64_t value) {
    if(head->is_leaf) 
        return head;

    auto& kv = head->internal_node->kv;
    int sz = kv.size();
    int i;
    for(i=1;i<sz;i+=2) {
        if(kv[i] >= value) break;
    }
    return findBlock((Node*)kv[i-1], value);
}


// Debug

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


// Insertion

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


void insert(Node* head, int64_t value, Node* &root) {
    head = findBlock(head, value);

    insertIntoLeaf(head, value, root);
}


// Print Tree

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


// Deletion

int calcKeys(int sz) {
    sz /= 2;
    return sz;
}

void mergeInternalNodes(Node* left, Node* right, int64_t mid) {
    left->internal_node->kv.push_back(mid);
    int cnt = 0;
    for(auto i: right->internal_node->kv) {
        if(cnt % 2 == 0) {
            ((Node*)i)->parent = left;
        }
        left->internal_node->kv.push_back(i);
        ++cnt;
    }
    delete right;
}

void deleteInternal(Node* head, int64_t key, int index, Node* &root) {
    auto& kv = head->internal_node->kv;
    int sz = kv.size();

    for(int i=index;i<sz-2;++i) {
        kv[i] = kv[i+2];
    }
    kv.resize(sz - 2);

    
    if(kv.size() == 1 && head->parent == nullptr) {
        int64_t child = kv[0];
        delete root->internal_node;
        delete root;
        root = (Node*)child;
        root->parent = 0;
        return;
    }
    if(head->parent == nullptr || calcKeys(sz-2) >= MIN_DATA_LEN) {
        return;
    }


    Node *leftSibling = nullptr, *rightSibling = nullptr, *parent = head->parent;


    auto& parkv = parent->internal_node->kv;
    int parsz = parkv.size();

    int i;
    for(i=1;i<parsz;i+=2) {
        if(parkv[i] >= key) break;
    }
    i -= 1;

    if(i-2 >= 0) leftSibling = (Node*)parkv[i-2];
    if(i+2 < parsz) rightSibling = (Node*)parkv[i+2];



    if(leftSibling != nullptr && calcKeys(leftSibling->internal_node->kv.size()) > MIN_DATA_LEN) {
        auto& leftkv = leftSibling->internal_node->kv;
        kv.insert(kv.begin(), parent->internal_node->kv[i-1]);
        kv.insert(kv.begin(), leftkv.back());
        ((Node*)leftkv.back())->parent = head;
        leftkv.pop_back();
        parent->internal_node->kv[i-1] = leftkv.back();
        leftkv.pop_back(); 
    }
    else if(rightSibling != nullptr && calcKeys(rightSibling->internal_node->kv.size()) >  MIN_DATA_LEN) {
        kv.push_back(parent->internal_node->kv[i+1]);
        kv.push_back(rightSibling->internal_node->kv.front());
        ((Node*)rightSibling->internal_node->kv.front())->parent = head;
        rightSibling->internal_node->kv.erase(rightSibling->internal_node->kv.begin());
        parent->internal_node->kv[i+1] = rightSibling->internal_node->kv.front();
        rightSibling->internal_node->kv.erase(rightSibling->internal_node->kv.begin());
    }
    else if(leftSibling != nullptr) {
        mergeInternalNodes(leftSibling, head, parkv[i-1]);
        deleteInternal(parent, parkv[i-1], i-1, root);
    }
    else if(rightSibling != nullptr) {
        mergeInternalNodes(head, rightSibling, parkv[i+1]);
        deleteInternal(parent, parkv[i+1], i+1, root);
    }
}

void mergeLeafNodes(Node* left, Node* right) {
    for(auto i: right->leaf_node->data) {
        left->leaf_node->data.push_back(i);
    }
    delete right;
}

void deleteLeaf(Node* head, int value, Node* &root) {
    auto& data = head->leaf_node->data;
    auto it = find(data.begin(), data.end(), value);
    if(it == data.end()) {
        cout << "Error: key does not exist\n";
        return;
    }
    data.erase(it);
    int sz = data.size();


    // The min and max data constraints do not apply to the root node
    if(head->parent == nullptr || sz >= MIN_DATA_LEN) {
        return;
    }

    Node *leftSibling = nullptr, *rightSibling = nullptr, *parent = head->parent;

    auto& parkv = parent->internal_node->kv;
    int parsz = parkv.size();

    int i;
    for(i=1;i<parsz;i+=2) {
        if(parkv[i] >= value) break;
    }

    i -= 1;
    if(i-2 >= 0) leftSibling = (Node*)parkv[i-2];
    if(i+2 < parsz) rightSibling = (Node*)parkv[i+2];


    if(leftSibling != nullptr && leftSibling->leaf_node->data.size() > MIN_DATA_LEN) {
        data.insert(data.begin(), leftSibling->leaf_node->data.back());
        leftSibling->leaf_node->data.pop_back();
        parkv[i-1] = leftSibling->leaf_node->data.back();
    }
    else if(rightSibling != nullptr && rightSibling->leaf_node->data.size() > MIN_DATA_LEN) {
        parkv[i+1] = rightSibling->leaf_node->data.front();
        data.push_back(parkv[i+1]);
        rightSibling->leaf_node->data.erase(rightSibling->leaf_node->data.begin());
    }
    else if(leftSibling != nullptr) {
        mergeLeafNodes(leftSibling, head);
        deleteInternal(parent, parkv[i-1], i-1, root);
    }
    else if(rightSibling != nullptr) {
        mergeLeafNodes(head, rightSibling);
        deleteInternal(parent, parkv[i+1], i+1, root);
    }
}

void deleteRecord(Node* head, int value, Node* &root) {
    head = findBlock(head, value);
    deleteLeaf(head, value, root);
}




int main(int argc, char* argv[]) {
    Node* root = new Node();
    root->is_leaf = 1;
    root->parent = NULL;
    root->row_len = DATA_LEN;
    root->leaf_node = new LeafNode();

    // vector<int> nums = {5, 12, 1, 2, 18 ,21};
    vector<int> nums = {5, 21, 16, 1, 6 ,2, 7, 9, 12, 18, 0};

    vector<int> v;
    for(int i=10; i<=530; i+=10) {
        v.push_back(i);
        insert(root, i, root);
    }
    print(root);

    cout << "--------------------------\n";

    while(v.size()) {
        int m = v.size();
        int ind = rand() % m;
        cout << "DELETE: " << v[ind] << "\n";

        deleteRecord(root, v[ind], root);
        print(root);
        v.erase(v.begin()+ind);
    }

    cout << "\n";

    return 0;
}