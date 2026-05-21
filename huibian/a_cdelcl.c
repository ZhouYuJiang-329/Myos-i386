int __attribute__((cdecl)) add(int a, int b)
{
    int x = 10;
    return a + b;
}

int main() {
    add(1, 2);
    return 0;
}

//gcc -m32 -S a_cdelcl.c -o acdecl.s