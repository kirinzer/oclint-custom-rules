#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"

using namespace std;
using namespace clang;
using namespace oclint;

class KirinzerTestRule : public AbstractASTVisitorRule<KirinzerTestRule>
{
public:
    virtual const string name() const override
    {
        return "if else format";
    }

    virtual int priority() const override
    {
        return 2;
    }

    virtual const string category() const override
    {
        return "controversial";
    }

#ifdef DOCGEN
    virtual const std::string since() const override
    {
        return "20.11";
    }

    virtual const std::string description() const override
    {
        return "用于检查 if else 条件分支中的括号是否符合编码规范";
    }

    virtual const std::string example() const override
    {
        return R"rst(
.. code-block:: cpp

        void example()
        {
        int a = 1;
        if(a > 0) { // (左侧无空格或换行不合规
        a = 10;
        }
        
        if (a > 0){ // )右侧无空格或换行不合规
        a = 10;
        }
        
        if (a > 0)
        {
        a = 10;
        }else { // }右侧无空格或换行不合规
        a = -1;
        }
        
        if (a > 0)
        {
        a = 10;
        } else{ // {左侧无空格或换行不合规
        a = -1;
        }
        }
        )rst";
    }

#endif
    
    bool VisitIfStmt(IfStmt *node)
    {
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        
        SourceLocation begin = node->getIfLoc();
        SourceLocation elseLoc = node->getElseLoc();
        SourceLocation end = node->getEndLoc();
        
        int length = sourceManager->getFileOffset(end) - sourceManager->getFileOffset(begin) + 1; // 计算该节点源码的长度
        string sourceCode = StringRef(sourceManager->getCharacterData(begin), length).str(); // 从起始位置按指定长度读取字符数据
//        printf("%s\n", sourceCode.c_str());
        
        // 检查 if 左括号
        std::size_t found = sourceCode.find("if (");
        if (found==std::string::npos) {
//            printf("if ( 格式不正确\n");
            AppendToViolationSet(node, Description());
        }
        
        // 检查 if 右括号
        found = sourceCode.find(") {");
        if (found==std::string::npos) {
            found = sourceCode.find(")\n");
            if (found ==std::string::npos) {
//                printf("if 右括号 格式不正确\n");
                AppendToViolationSet(node, Description());
            }
        }
        
        // 没有 else 分支就不再进行检查
        if (!elseLoc.isValid()) {
            return true;
        }
        
        // 检查 else 左括号
        found = sourceCode.find("} else");
        if (found==std::string::npos) {
            found = sourceCode.find("}\n");
            if (found==std::string::npos) {
//                printf("} else 格式不正确\n");
                AppendToViolationSet(node, Description());
            }
        }
        
        // 检查 else 右括号
        found = sourceCode.find("else {");
        if (found==std::string::npos) {
            found = sourceCode.find("else\n");
            if (found==std::string::npos) {
//                printf("else { 格式不正确\n");
                AppendToViolationSet(node, Description());
            }
        }
        
        return true;
    }
    
    // 将违例信息追加进结果集
    bool AppendToViolationSet(IfStmt *node, string description) {
        addViolation(node, this, description);
    }
    
    string Description() {
        return "格式不正确";
    }
};

static RuleSet rules(new KirinzerTestRule());
