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
        struct node;//节点 没多少函数 全写cpp里
        enum class valueType
        {
            Empty,
            Str,
            I64,
            F64
        };
        struct nodeView//查询节点
        {
            node* ptr;
            nodeView(node *p) : ptr(p) {}
            bool isExist(const std::string &key){return ptr->obj.count(key);}//查找是否有这个孩子节点
            nodeView get(std::string& key);//返回孩子节点
            nodeView operator[](size_t index);
            nodeView value(std::string& path);//路径解析

            //面向对象的
            bool is_string() const { return ptr->is_string(); }
            bool is_ll() const { return ptr->is_ll(); }
            bool is_object() const { return ptr->is_object(); }
            bool is_array() const  { return ptr->is_array();  }
            std::string str()      { return std::get<std::string>(ptr->val);}//节点的值(string类型)
            long long i64()          { return std::get<long long>(ptr->val);}//节点的值(long long类型)
            double f64()          { return std::get<double>(ptr->val);}//节点的值(double类型)
        };
        
        
    }
}