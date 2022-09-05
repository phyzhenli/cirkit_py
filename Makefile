CC = g++
#CFLAGS = -std=c++11 -DABC_USE_STDINT_H=1 -fPIC
CFLAGS  = -O2 -DNDEBUG -std=c++17 -Wno-unknown-pragmas
CFLAGS += -DFMT_HEADER_ONLY=1 -DDISABLE_NAUTY=1 -DLIN64=1 -DABC_NAMESPACE=pabc -DABC_NO_USE_READLINE=1

LIB_INC  = -Ilib/alice -Ilib/any -Ilib/cli11 -Ilib/mockturtle/include
LIB_INC += -Ilib/mockturtle/lib/abcesop -Ilib/mockturtle/lib/abcsat -Ilib/mockturtle/lib/bill -Ilib/mockturtle/lib/fmt
LIB_INC += -Ilib/mockturtle/lib/json -Ilib/mockturtle/lib/kitty -Ilib/mockturtle/lib/lorina
LIB_INC += -Ilib/mockturtle/lib/percy -Ilib/mockturtle/lib/rang -Ilib/mockturtle/lib/parallel_hashmap

INCLUDES = -Icli $(LIB_INC)

PYTHON_INC = $(shell python3 -m pybind11 --includes)

ABC_SRC  = lib/mockturtle/lib/abcesop/exor.cpp lib/mockturtle/lib/abcesop/exorBits.cpp lib/mockturtle/lib/abcesop/exorCubes.cpp
ABC_SRC += lib/mockturtle/lib/abcesop/exorLink.cpp lib/mockturtle/lib/abcesop/exorList.cpp lib/mockturtle/lib/abcesop/exorUtil.cpp
ABC_SRC += lib/mockturtle/lib/abcsat/satSolver.cpp lib/mockturtle/lib/abcsat/satStore.cpp

CLI_SRC  = cirkit.cpp $(ABC_SRC)
CLI_OBJS = $(CLI_SRC:.cpp=.o)
CLI_TARGET = cirkit

PY_SRC  = cirkit_py.cpp $(ABC_SRC)
PY_OBJS = $(PY_SRC:.cpp=.po)
PY_TARGET = cirkit_py$(shell python3-config --extension-suffix)

ALL: $(CLI_TARGET) $(PY_TARGET)

$(CLI_TARGET): $(CLI_OBJS)
	$(CC) $^ -o $@

$(PY_TARGET): $(PY_OBJS)
	$(CC) -shared -Wl,--export-dynamic $^ -o $@

.PHONY:clean
clean:
	rm -rf $(CLI_TARGET) $(PY_TARGET) $(CLI_OBJS) $(PY_OBJS)


%.o:%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

%.po:%.cpp
	$(CC) $(CFLAGS) -DALICE_PYTHON -fPIC $(INCLUDES) $(PYTHON_INC) -o $@ -c $<
