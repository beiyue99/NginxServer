

#用于检查变量 DEBUG 是否等于字符串 "true"
ifeq ($(DEBUG),true)
CC = gcc -g
VERSION = debug
else
CC = gcc
VERSION = release
endif


# $(wildcard *.c)表示扫描当前目录下所有.c文件
SRCS = $(wildcard *.c)

   
OBJS = $(SRCS:.c=.o)     # 将 SRCS 中的每个.c文件替换为.o，结果保存在变量 OBJS 中

#把字符串中的.c替换为.d
DEPS = $(SRCS:.c=.d)

#将变量 BIN 中的每个单词（通常是文件名）都添加了 $(BUILD_ROOT)/ 这个前缀例如，
#这种方式有助于将生成的文件放置在指定的目录中，而不是当前目录。
# := 在解析阶段直接赋值常量字符串【立即展开】，而 = 在运行阶段，实际使用变量时再进行求值【延迟展开】
#如果使用 =，那么在 BIN 被使用时，$(BUILD_ROOT) 的值会被递归展开，
#而且如果 BUILD_ROOT 在后续的规则中发生变化，那么 BIN 的值也会相应地改变
BIN := $(addprefix $(BUILD_ROOT)/,$(BIN))

#定义存放ojb文件的目录，目录统一到一个位置才方便后续链接，不然整到各个子目录去，不好链接
LINK_OBJ_DIR = $(BUILD_ROOT)/app/link_obj
DEP_DIR = $(BUILD_ROOT)/app/dep

#-p是递归创建目录，没有就创建，有就不需要创建了
$(shell mkdir -p $(LINK_OBJ_DIR))
$(shell mkdir -p $(DEP_DIR))

#我们要把目标文件生成到上述目标文件目录去，利用函数addprefix增加个前缀
#处理后形如   /mnt/hgfs/linux/nginx/app/link_obj/ngx_signal.o
OBJS := $(addprefix $(LINK_OBJ_DIR)/,$(OBJS))
DEPS := $(addprefix $(DEP_DIR)/,$(DEPS))

#找到目录中的所有.o文件（编译出来的）
LINK_OBJ = $(wildcard $(LINK_OBJ_DIR)/*.o)
#部分 .c 文件是在 app 子目录中，而生成的 .o 文件会被放在 app/link_obj 目录下。在构建依赖关系时，即生成 .d 文件时，
#app目录下的 .o 文件还没有构建，因此在 LINK_OBJ 中缺少这些文件。
LINK_OBJ += $(OBJS)


#如下这行会是开始执行的入口，执行就找到依赖项$(BIN)去执行了，同时，这里也依赖了$(DEPS)，这样就会生成很多.d文件了
all:$(DEPS) $(OBJS) $(BIN)

#这里是诸多.d文件被包含进来，每个.d文件里都记录着一个.o文件所依赖哪些.c和.h文件。内容诸如 nginx.o: nginx.c ngx_func.h
#我们做这个的最终目的说白了是，即便.h被修改了，也要让make重新编译我们的工程，否则，你修改了.h，make不会重新编译，那是不行的
#有必要先判断这些文件是否存在，不然make可能会报一些.d文件找不到
ifneq ("$(wildcard $(DEPS))", "")   
include $(DEPS)  
endif

#$(LINK_OBJ_DIR)/%.o 是规则的目标，表示所有放在 $(LINK_OBJ_DIR) 目录下的 .o 文件。: 后面是规则的依赖项，
#这里使用了 %.c 模式，表示所有匹配这个模式的 .c 文件都是依赖项。 $^ 表示所有依赖项，这里就是所有匹配到的 .c 文件。
#-c 表示只进行编译而不进行链接。$(filter %.c,$^) 用于过滤出所有依赖项中的 .c 文件，
#因为 $^ 包含了所有的依赖项，有可能其中包含了一些不是 .c 文件的东西
$(BIN):$(LINK_OBJ)
	@echo "------------------------build $(VERSION) mode--------------------------------!!!"
	$(CC) -o $@ $^
$(LINK_OBJ_DIR)/%.o:%.c
	$(CC) -I $(INCLUDE_PATH) -o $@ -c $(filter %.c,$^)




#我们现在希望当修改一个.h时，也能够让make自动重新编译我们的项目，所以，我们需要指明让.o依赖于.h文件
#那一个.o依赖于哪些.h文件，我们可以用“gcc -MM c程序文件名” 来获得这些依赖信息并重定向保存到.d文件中
#.d文件中的内容可能形如：nginx.o: nginx.c ngx_func.h
#%.d:%.c
$(DEP_DIR)/%.d:%.c
#gcc -MM $^ > $@
#.d文件中的内容形如：nginx.o: nginx.c ngx_func.h ../signal/ngx_signal.h，但现在的问题是我们的.o文件已经放到了专门的目录
# 所以我们要正确指明.o文件路径这样，对应的.h,.c修改后，make时才能发现，这里需要用到sed文本处理工具和一些正则表达式语法，不必深究
#gcc -MM $^ | sed 's,\(.*\)\.o[ :]*,$(LINK_OBJ_DIR)/\1.o:,g' > $@
#echo 中 -n表示后续追加不换行
	echo -n $(LINK_OBJ_DIR)/ > $@
#	gcc -MM $^ | sed 's/^/$(LINK_OBJ_DIR)&/g' > $@
#  >>表示追加
	gcc -I$(INCLUDE_PATH) -MM $^ >> $@

#上行处理后，.d文件中内容应该就如：/mnt/hgfs/linux/nginx/app/link_obj/nginx.o: nginx.c ngx_func.h ../signal/ngx_signal.h

#clean:			
#rm 的-f参数是不提示强制删除
#可能gcc会产生.gch这个优化编译速度文件
#	rm -f $(BIN) $(OBJS) $(DEPS) *.gch
#----------------------------------------------------------------nend------------------





