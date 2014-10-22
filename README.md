membuf
======

`membuf_t` is an auto-growable continuous in-memory buffer. It also support "local buffer" to use stack memory efficiently.

    membuf_t buf; char on_stack_buffer[16];
    membuf_init_local(&buf, on_stack_buffer, sizeof(on_stack_buffer));
    membuf_append_text(&buf, "0123456789", 10); // uses stack buffer
    membuf_append_text(&buf, "ABCDEF", 6); // still uses stack buffer
    membuf_append_zeros(&buf, 1); // now it uses heap buffer
    printf("%s\n", buf.data);
    membuf_uninit(&buf);

