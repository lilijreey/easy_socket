#pragma once

#include <stdint.h>
//definition client <-> server protocol struct

enum {
    PROTO_MAX_BODY_SIZE = 1<<10 ,
    PROTO_MAGIC = 0x33445566,
    PROTO_ADD_CMD=1,
};

struct proto_head_t
{
    bool is_valid_head() {return true;}
    uint32_t magic;
    uint32_t body_len;
    uint8_t  cmd;
    uint8_t  ver;

} __attribute__((packed));



//client -> server
//exection + operation
struct proto_add_request_t
{
    proto_head_t head;
    uint32_t pair_count;
    struct {
        int32_t num1;
        int32_t num2;
    } pair[0];

}__attribute__((packed));;

//return add result
struct proto_add_response_t
{
    proto_head_t head;
    uint32_t count;
    uint64_t resluts[0];

};
