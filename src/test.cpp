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
    cgen::Function function;

    function.name = "main";
    function.return_type = cgen::i32();

    function.parameters.push_back(cgen::decl_local("a0", cgen::i32()));
    function.parameters.push_back(cgen::array_of(cgen::decl_local("a1", cgen::pointer_of(cgen::i8()))));

    function.body = std::make_unique<cgen::Block>();
    auto block = function.body->as<cgen::Block>();

    block->push(std::make_unique<cgen::DeclLocal>());

    auto local = block->nodes[0]->as<cgen::DeclLocal>();
    local->name = "l0";
    local->type = cgen::u8();

    block->push(std::make_unique<cgen::Return>());

    auto ret = block->nodes[1]->as<cgen::Return>();
    ret->node = cgen::literal(0);

    cgen::CodeGenVisitor visitor;
    assert(function.accept(&visitor) == "int main(int a0, char* a1[]){unsigned char l0;return 0;}");
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

void test()
{
    RUN_TEST(test_primitive);
    RUN_TEST(test_declare_local);
    RUN_TEST(test_block);
    RUN_TEST(test_function);
    RUN_TEST(test_struct);
    RUN_TEST(test_call);
}

int main()
{
    test();
    return 0;
}
