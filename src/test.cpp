#include <iostream>
#include <cassert>

#define CGEN_IMPLEMENTATION
#include "cgen.hpp"

#define RUN_TEST(test)                                      \
    std::cout << "Running " << #test << "..." << std::endl; \
    test();                                                 \
    std::cout << #test << " passed." << std::endl;

void test_primitive()
{
    cgen::CodeGenVisitor visitor;

    assert(cgen::u8()->accept(&visitor) == "unsigned char");
    assert(cgen::u16()->accept(&visitor) == "unsigned short");
    assert(cgen::u32()->accept(&visitor) == "unsigned int");
    assert(cgen::u64()->accept(&visitor) == "unsigned long");
    assert(cgen::i8()->accept(&visitor) == "char");
    assert(cgen::i16()->accept(&visitor) == "short");
    assert(cgen::i32()->accept(&visitor) == "int");
    assert(cgen::i64()->accept(&visitor) == "long");
    assert(cgen::f32()->accept(&visitor) == "float");
    assert(cgen::f64()->accept(&visitor) == "double");
}

void test_declare_local()
{
    cgen::Static static_;

    static_.node = std::make_unique<cgen::DeclLocal>();

    auto local = static_.node->as<cgen::DeclLocal>();
    local->name = "x";
    local->type = cgen::u8();

    cgen::CodeGenVisitor visitor;
    std::string result = static_.accept(&visitor);
    assert(result == "static unsigned char x");
}

void test_block()
{
    cgen::Block block;

    block.push(std::make_unique<cgen::DeclLocal>());

    auto local = block.nodes[0]->as<cgen::DeclLocal>();
    local->name = "x";
    local->type = pointer_of(cgen::u8());

    cgen::CodeGenVisitor visitor;

    std::string result = block.accept(&visitor);
    assert(result == "{unsigned char* x;}");
}

void test_function()
{
    cgen::Program program;

    {
        auto fn = std::make_unique<cgen::Function>();
        fn->name = "main";
        fn->return_type = cgen::i32();

        fn->parameters.push_back(cgen::decl_local("a0", cgen::i32()));
        fn->parameters.push_back(cgen::array_of(cgen::decl_local("a1", cgen::pointer_of(cgen::i8()))));

        {
            auto body = std::make_unique<cgen::Block>();

            {
                auto decl = std::make_unique<cgen::DeclLocal>();
                decl->name = "l0";
                decl->type = cgen::u8();

                body->push(std::move(decl));

                auto ret = std::make_unique<cgen::Return>();
                ret->node = cgen::literal(0);

                body->push(std::move(ret));
            }

            fn->body = std::move(body);
        }

        program.nodes.push_back(std::move(fn));
    }

    {
        auto fn = std::make_unique<cgen::Function>();
        fn->name = "foo";
        fn->return_type = cgen::i32();
        fn->body = std::make_unique<cgen::Block>();

        program.nodes.push_back(std::move(fn));
    }

    cgen::CodeGenVisitor visitor;
    assert(program.accept(&visitor) == "int main(int a0, char* a1[]){unsigned char l0;return 0;};int foo(){};");
}

void test_struct()
{
    auto type = cgen::decl_type(
        "Point",
        cgen::decl_local("p0", cgen::i32()),
        cgen::decl_local("p1", cgen::i8()));

    cgen::CodeGenVisitor visitor;
    assert(type->accept(&visitor) == "struct Point{int p0;char p1;};");

    auto fild_p0 = cgen::field(cgen::local("a"), "p0");
    assert(fild_p0->accept(&visitor) == "a.p0");
}

void test_call()
{
    auto call = cgen::call(cgen::local("foo"), cgen::literal(1), cgen::literal(2));

    cgen::CodeGenVisitor visitor;
    assert(call->accept(&visitor) == "foo(1,2)");
}

void test_pointer_work()
{
    auto type = cgen::pointer_of(cgen::i32());
    cgen::CodeGenVisitor visitor;
    assert(type->accept(&visitor) == "int*");
}

int main()
{
    RUN_TEST(test_primitive);
    RUN_TEST(test_declare_local);
    RUN_TEST(test_block);
    RUN_TEST(test_function);
    RUN_TEST(test_struct);
    RUN_TEST(test_call);
}
