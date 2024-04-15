#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>

namespace cgen
{
    class Visitor;
    using AcceptResult = std::string;

    class Node
    {
    public:
        virtual AcceptResult accept(Visitor *visitor);

        template <typename T>
        inline T *as()
        {
            return dynamic_cast<T *>(this);
        }
    };

    class Program : public Node
    {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        AcceptResult accept(Visitor *visitor) override;

        void push(std::unique_ptr<Node> node)
        {
            nodes.push_back(std::move(node));
        }
    };

    class PointerOf : public Node
    {
    public:
        std::unique_ptr<Node> node;
        AcceptResult accept(Visitor *visitor) override;
    };

    class ArrayOf : public Node
    {
    public:
        std::unique_ptr<Node> node;
        size_t size = 0;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Primitive : public Node
    {
    public:
        enum Kind
        {
            i8,
            i16,
            i32,
            i64,
            u8,
            u16,
            u32,
            u64,
            f32,
            f64,
        } kind;

        Primitive(Kind kind) : kind(kind) {}

        AcceptResult accept(Visitor *visitor) override;
    };

    class Type : public Node
    {
    public:
        std::string name;
        AcceptResult accept(Visitor *visitor) override;

        inline std::unique_ptr<PointerOf> pointer_of()
        {
            auto ptr = std::make_unique<PointerOf>();
            ptr->node = std::move(std::unique_ptr<Node>(this));
            return ptr;
        }

        inline std::unique_ptr<ArrayOf> array_of(size_t size = 0)
        {
            auto arr = std::make_unique<ArrayOf>();
            arr->node = std::move(std::unique_ptr<Node>(this));
            arr->size = size;
            return arr;
        }
    };

    class Static : public Node
    {
    public:
        std::unique_ptr<Node> node;
        AcceptResult accept(Visitor *visitor) override;
    };

    template <typename T>
    class Literal : public Node
    {
    public:
        T value;
        AcceptResult accept(Visitor *visitor) override
        {
            return std::to_string(value);
        }
    };

    template <>
    class Literal<std::string> : public Node
    {
    public:
        std::string value;
        AcceptResult accept(Visitor *visitor) override
        {
            return "\"" + value + "\"";
        }
    };

    template <>
    class Literal<char> : public Node
    {
    public:
        char value;
        AcceptResult accept(Visitor *visitor) override
        {
            return "'" + std::string(1, value) + "'";
        }
    };

    class DeclLocal : public Node
    {
    public:
        std::string name;
        std::unique_ptr<Node> type;

        AcceptResult accept(Visitor *visitor) override;
    };

    class Assign : public Node
    {
    public:
        std::unique_ptr<Node> lhs;
        std::unique_ptr<Node> rhs;

        AcceptResult accept(Visitor *visitor) override;
    };

    class Block : public Node
    {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        AcceptResult accept(Visitor *visitor) override;

        void push(std::unique_ptr<Node> node)
        {
            nodes.push_back(std::move(node));
        }
    };

    class Function : public Node
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<Node>> parameters;
        std::unique_ptr<Node> return_type;
        std::unique_ptr<Node> body;

        AcceptResult accept(Visitor *visitor) override;
    };

    class Return : public Node
    {
    public:
        std::unique_ptr<Node> node;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Field : public Node
    {
    public:
        std::unique_ptr<Node> type;
        std::string name;
        AcceptResult accept(Visitor *visitor) override;
    };

    class DeclType : public Node
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<Node>> fields;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Deref : public Node
    {
    public:
        std::unique_ptr<Node> node;
        AcceptResult accept(Visitor *visitor) override;
    };

    class GetRef : public Node
    {
    public:
        std::unique_ptr<Node> node;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Local : public Node
    {
    public:
        std::string name;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Call : public Node
    {
    public:
        std::unique_ptr<Node> node;
        std::vector<std::unique_ptr<Node>> nodes;
        AcceptResult accept(Visitor *visitor) override;
    };

    class Visitor
    {
    public:
        virtual AcceptResult visit(class Node *node) = 0;
        virtual AcceptResult visit(class Program *node) = 0;
        virtual AcceptResult visit(class Primitive *node) = 0;
        virtual AcceptResult visit(class Type *node) = 0;
        virtual AcceptResult visit(class PointerOf *node) = 0;
        virtual AcceptResult visit(class ArrayOf *node) = 0;
        virtual AcceptResult visit(class Static *node) = 0;
        virtual AcceptResult visit(class DeclLocal *node) = 0;
        virtual AcceptResult visit(class Block *node) = 0;
        virtual AcceptResult visit(class Function *node) = 0;
        virtual AcceptResult visit(class Return *node) = 0;
        virtual AcceptResult visit(class Assign *node) = 0;
        virtual AcceptResult visit(class Field *node) = 0;
        virtual AcceptResult visit(class Deref *node) = 0;
        virtual AcceptResult visit(class GetRef *node) = 0;
        virtual AcceptResult visit(class DeclType *node) = 0;
        virtual AcceptResult visit(class Local *node) = 0;
        virtual AcceptResult visit(class Call *node) = 0;
    };

    class CodeGenVisitor : public Visitor
    {
    public:
        AcceptResult visit(Node *node) override;
        AcceptResult visit(Program *node) override;
        AcceptResult visit(Primitive *node) override;
        AcceptResult visit(Type *node) override;
        AcceptResult visit(PointerOf *node) override;
        AcceptResult visit(ArrayOf *node) override;
        AcceptResult visit(Static *node) override;
        AcceptResult visit(DeclLocal *node) override;
        AcceptResult visit(Block *node) override;
        AcceptResult visit(Function *node) override;
        AcceptResult visit(Return *node) override;
        AcceptResult visit(Assign *node) override;
        AcceptResult visit(Field *node) override;
        AcceptResult visit(Deref *node) override;
        AcceptResult visit(GetRef *node) override;
        AcceptResult visit(DeclType *node) override;
        AcceptResult visit(Local *node) override;
        AcceptResult visit(Call *node) override;
    };

#define __CGEN_PRIMITIVE(name) \
    inline std::unique_ptr<Node> name() { return std::unique_ptr<Node>((Node *)new Primitive(Primitive::name)); }

    __CGEN_PRIMITIVE(i8)
    __CGEN_PRIMITIVE(i16)
    __CGEN_PRIMITIVE(i32)
    __CGEN_PRIMITIVE(i64)
    __CGEN_PRIMITIVE(u8)
    __CGEN_PRIMITIVE(u16)
    __CGEN_PRIMITIVE(u32)
    __CGEN_PRIMITIVE(u64)
    __CGEN_PRIMITIVE(f32)
    __CGEN_PRIMITIVE(f64)

    inline std::unique_ptr<Node> pointer_of(std::unique_ptr<Node> node)
    {
        auto ptr = new PointerOf();
        ptr->node = std::move(node);
        return std::unique_ptr<Node>((Node *)ptr);
    }

    inline std::unique_ptr<Node> array_of(std::unique_ptr<Node> node, size_t size = 0)
    {
        auto arr = new ArrayOf();
        arr->node = std::move(node);
        arr->size = size;
        return std::unique_ptr<Node>((Node *)arr);
    }

    inline std::unique_ptr<Node> decl_local(std::string name, std::unique_ptr<Node> type)
    {
        auto dl = new DeclLocal();
        dl->name = name;
        dl->type = std::move(type);
        return std::unique_ptr<Node>((Node *)dl);
    }

    template <typename T>
    inline std::unique_ptr<Node> literal(T value)
    {
        auto lit = std::make_unique<Literal<T>>();
        lit->value = value;
        return lit;
    }

    inline std::unique_ptr<Node> local(std::string name)
    {
        auto l = std::make_unique<Local>();
        l->name = name;
        return l;
    }

    inline std::unique_ptr<Node> field(std::unique_ptr<Node> type, std::string name)
    {
        auto f = std::make_unique<Field>();
        f->type = std::move(type);
        f->name = name;
        return f;
    }

    template <typename... fields>
    inline std::unique_ptr<Node> decl_type(std::string name, fields... fields_)
    {
        auto dt = std::make_unique<DeclType>();
        dt->name = name;
        (dt->fields.push_back(std::move(fields_)), ...);
        return dt;
    }

    template <typename... nodes>
    inline std::unique_ptr<Node> call(std::unique_ptr<Node> node, nodes... nodes_)
    {
        auto c = std::make_unique<Call>();
        c->node = std::move(node);
        (c->nodes.push_back(std::move(nodes_)), ...);
        return c;
    }

    inline std::unique_ptr<Node> get_ref(std::unique_ptr<Node> node)
    {
        auto r = std::make_unique<GetRef>();
        r->node = std::move(node);
        return r;
    }
} // namespace cgen

#ifdef CGEN_IMPLEMENTATION

namespace cgen
{
    AcceptResult Node::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Program::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Primitive::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Type::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Assign::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Block::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Function::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Static::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult DeclLocal::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult PointerOf::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult ArrayOf::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Return::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Field::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Deref::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult DeclType::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Local::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult Call::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult GetRef::accept(Visitor *visitor)
    {
        return visitor->visit(this);
    }

    AcceptResult CodeGenVisitor::visit(Node *node)
    {
        return node->accept(this);
    }

    AcceptResult CodeGenVisitor::visit(Program *node)
    {
        std::stringstream ss;

        for (auto &n : node->nodes)
        {
            ss << n->accept(this) << ";";
        }

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Primitive *node)
    {
        std::stringstream ss;

        switch (node->kind)
        {
        case Primitive::i8:
            ss << "char";
            break;
        case Primitive::i16:
            ss << "short";
            break;
        case Primitive::i32:
            ss << "int";
            break;
        case Primitive::i64:
            ss << "long";
            break;
        case Primitive::u8:
            ss << "unsigned char";
            break;
        case Primitive::u16:
            ss << "unsigned short";
            break;
        case Primitive::u32:
            ss << "unsigned int";
            break;
        case Primitive::u64:
            ss << "unsigned long";
            break;
        case Primitive::f32:
            ss << "float";
            break;
        case Primitive::f64:
            ss << "double";
            break;
        }

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Type *node)
    {
        return "struct " + node->name;
    }

    AcceptResult CodeGenVisitor::visit(PointerOf *node)
    {
        return node->node->accept(this) + "*";
    }

    AcceptResult CodeGenVisitor::visit(ArrayOf *node)
    {
        std::stringstream ss;

        ss << node->node->accept(this) << "[";
        if (node->size > 0)
        {
            ss << node->size;
        }
        ss << "]";

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Static *node)
    {
        return "static " + node->node->accept(this);
    }

    AcceptResult CodeGenVisitor::visit(DeclLocal *node)
    {
        return node->type->accept(this) + " " + node->name;
    }

    AcceptResult CodeGenVisitor::visit(Block *node)
    {
        std::stringstream ss;

        ss << "{";

        for (auto &n : node->nodes)
        {
            ss << n->accept(this) << ";";
        }

        ss << "}";

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Function *node)
    {
        std::stringstream ss;

        ss << node->return_type->accept(this) << " " << node->name << "(";

        for (size_t i = 0; i < node->parameters.size(); i++)
        {
            ss << node->parameters[i]->accept(this);

            if (i < node->parameters.size() - 1)
            {
                ss << ", ";
            }
        }

        ss << ")";
        ss << node->body->accept(this);

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Return *node)
    {
        return "return " + node->node->accept(this);
    }

    AcceptResult CodeGenVisitor::visit(Assign *node)
    {
        return node->lhs->accept(this) + " = " + node->rhs->accept(this);
    }

    AcceptResult CodeGenVisitor::visit(Field *node)
    {
        return node->type->accept(this) + "." + node->name;
    }

    AcceptResult CodeGenVisitor::visit(Deref *node)
    {
        return "(*" + node->node->accept(this) + ")";
    }

    AcceptResult CodeGenVisitor::visit(DeclType *node)
    {
        std::stringstream ss;

        ss << "struct " << node->name << "{";

        for (size_t i = 0; i < node->fields.size(); i++)
        {
            ss << node->fields[i]->accept(this);
            ss << ";";
        }

        ss << "};";

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(Local *node)
    {
        return node->name;
    }

    AcceptResult CodeGenVisitor::visit(Call *node)
    {
        std::stringstream ss;

        ss << node->node->accept(this) << "(";

        for (size_t i = 0; i < node->nodes.size(); i++)
        {
            ss << node->nodes[i]->accept(this);

            if (i < node->nodes.size() - 1)
            {
                ss << ",";
            }
        }

        ss << ")";

        return ss.str();
    }

    AcceptResult CodeGenVisitor::visit(GetRef *node)
    {
        return "(&" + node->node->accept(this) + ")";
    }

} // namespace1

#endif // CGEN_IMPLEMENTATION