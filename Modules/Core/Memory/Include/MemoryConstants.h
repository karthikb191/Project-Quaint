#ifndef _H_MEMORY_CONSTANTS
#define _H_MEMORY_CONSTANTS

namespace Quaint
{
    /*
    *   This class is used to generate memory context information at compile time
    *   TODO: Make this a generated file based on config later
    */
    #define MAX_MEMORY_CONTEXTS 50

    #define BOOT_MEMORY_NAME "DEFAULT"
    #define DEFAULT_MEMORY_NAME BOOT_MEMORY_NAME

    #define BOOT_MEMORY_SIZE  50 * 1024 * 1024 //50Mib

    //Bolt(Graphics) Memory Partitions
    #define BOLT_MEMORY_PARTITION "BOLT_MEMORY"
    #define BOLT_MEMORY_SIZE 250 * 1024 * 1024 // 250 Mb
    #define BOLT_MEMORY_INDEX 1

    //Other Memory partitions
    #define MEDIA_MEMORY_NAME "VIDEO"
    #define MEDIA_MEMORY_SIZE 50 * 1024 * 1024 //50 Mib
    #define MEDIA_MEMORY_INDEX 2

    #define DEFAULT_ALIGNMENT 8
    #define PADDING_TYPE      size_t
    #define PADDING_INFO_SIZE sizeof(size_t)
}

#endif //_H_MEMORY_CONSTANTS