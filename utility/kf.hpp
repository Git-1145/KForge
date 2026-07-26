#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <memory>
#include <windows.h>
#include <chrono>

//#define ll long long

namespace KF::UTI
{
    namespace FIO
    {
        enum class valueType
        {
            Empty,
            Str,
            I64,
            F64
        };
        struct node;
        using nodePtr = std::unique_ptr<node>;
        using objMap = std::unordered_map<std::string, nodePtr>;
        struct node//节点
        {
            objMap obj;//{} 孩子有哪些
            std::vector<node> arr;//()数组 node退化成val 
            valueType type=valueType::Empty;
            std::variant<std::string, long long, double> val;//值
            //构造
            node()=default;
            node(node&&)=default;
            node& operator= (node&&)=default;
            //查询类型
            bool is_object() const { return !obj.empty(); }
            bool is_array()  const { return !arr.empty(); }
            bool is_string() const { return type == valueType::Str; }
            bool is_ll() const { return type == valueType::I64;}
            bool is_db() const { return type == valueType::F64;}
            
        };
        
        struct nodeView//查询节点
        {
            node* ptr;
            nodeView(node *p) : ptr(p) {}
            bool isExist(const std::string &key) const { return ptr->obj.count(key) != 0; }//查找是否有这个孩子节点
            nodeView get(const std::string& key);//返回孩子节点
            nodeView operator[](size_t index);
            nodeView value(const std::string& path);//路径解析

            //面向对象的
            bool is_string() const { return ptr->is_string(); }
            bool is_ll() const { return ptr->is_ll(); }
            bool is_object() const { return ptr->is_object(); }
            bool is_array() const  { return ptr->is_array();  }
            std::string str()    { return std::get<std::string>(ptr->val);}//节点的值(string类型)
            long long i64()          { return std::get<long long>(ptr->val);}//节点的值(long long类型)
            double f64()          { return std::get<double>(ptr->val);}//节点的值(double类型)
        };

        void open(const std::string& path);
        nodeView read(const std::string& topKey);
    }
}