set(SRCS
	collection/Array.h
	collection/Array2DView.h
	collection/Array3DView.h
	collection/BitSet.h
	collection/Buffer.h
	collection/BufferView.h
	collection/ConcurrentDynamicArray.h
	collection/ConcurrentQueue.h
	collection/ConcurrentPriorityQueue.h
	collection/ConcurrentSet.h
	collection/DynamicArray.h
	collection/DynamicMap.h
	collection/DynamicStringMap.h
	collection/Functions.h
	collection/List.h
	collection/Map.h collection/Map.cpp
	collection/Set.h
	collection/Stack.h
	collection/StringMap.h
	collection/StringSet.h
	collection/Vector.h

	concurrent/Atomic.cpp concurrent/Atomic.h
	concurrent/Concurrency.h concurrent/Concurrency.cpp
	concurrent/ConditionVariable.h concurrent/ConditionVariable.cpp
	concurrent/Lock.cpp concurrent/Lock.h
	concurrent/ReadWriteLock.cpp concurrent/ReadWriteLock.h
	concurrent/Semaphore.cpp concurrent/Semaphore.h
	concurrent/ThreadPool.cpp concurrent/ThreadPool.h
	concurrent/Thread.cpp concurrent/Thread.h

	Algorithm.h
	Alphanumeric.h
	ArrayLength.h
	Assert.cpp Assert.h
	BindingContext.cpp BindingContext.h
	Bits.h
	Color.cpp Color.h
	Common.cpp Common.h
	CMYK.cpp CMYK.h
	DirtyState.h
	Endian.h
	Enum.h
	FixedString.h
	FourCC.h
	ConfigVar.h
	GLM.cpp GLM.h
	GLMConst.h
	Hash.cpp Hash.h
	IComponent.h
	Log.cpp Log.h
	MD5.cpp MD5.h
	NonCopyable.h
	Optional.h
	Pair.h
	Path.cpp Path.h
	PoolAllocator.h
	Process.cpp Process.h
	RGBA.h RGBA.cpp
	SharedPtr.h
	Singleton.h
	StandardLib.h
	String.cpp String.h
	StringUtil.cpp StringUtil.h
	TimeProvider.h TimeProvider.cpp
	TimedValue.h
	Tokenizer.h Tokenizer.cpp
	Trace.cpp Trace.h
	Tuple.h
	UTF8.cpp UTF8.h
	Var.cpp Var.h
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracy/public/TracyClient.cpp)
	list(APPEND SRCS tracy/public/TracyClient.cpp)
	message(STATUS "Building with tracy enabled")
else()
	message(STATUS "Building with tracy disabled")
endif()

set(LIB core)

set(LIBS sdl3 glm ${CMAKE_DL_LIBS})

if (USE_STACKTRACES)
	list(APPEND LIBS backward)
endif()

engine_add_module(TARGET ${LIB} SRCS ${SRCS} DEPENDENCIES ${LIBS})
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracy/public/TracyClient.cpp)
	target_compile_definitions(${LIB} PUBLIC TRACY_ENABLE TRACY_ON_DEMAND)
endif()
target_compile_definitions(${LIB} PRIVATE MINIZ_NO_STDIO)

if (WINDOWS)
	# All this windows.h insanities ... giving up on this module
	set_target_properties(${LIB} PROPERTIES UNITY_BUILD FALSE)
endif()

if (USE_STACKTRACES)
	target_compile_definitions(${LIB} PRIVATE HAVE_BACKWARD)
endif()

set(TEST_SRCS
	tests/TestHelper.h
	tests/AlgorithmTest.cpp
	tests/AlphanumericTest.cpp
	tests/ArrayTest.cpp
	tests/BitsTest.cpp
	tests/BitSetTest.cpp
	tests/BufferTest.cpp
	tests/ColorTest.cpp
	tests/ConcurrentDynamicArrayTest.cpp
	tests/ConcurrentPriorityQueueTest.cpp
	tests/ConcurrentQueueTest.cpp
	tests/CoreTest.cpp
	tests/DynamicArrayTest.cpp
	tests/HashTest.cpp
	tests/ListTest.cpp
	tests/MapTest.cpp
	tests/DynamicMapTest.cpp
	tests/MD5Test.cpp
	tests/OptionalTest.cpp
	tests/PathTest.cpp
	tests/PoolAllocatorTest.cpp
	tests/QueueTest.cpp
	tests/ReadWriteLockTest.cpp
	tests/RingBufferTest.cpp
	tests/SharedPtrTest.cpp
	tests/StackTest.cpp
	tests/StringTest.cpp
	tests/StringUtilTest.cpp
	tests/ThreadPoolTest.cpp
	tests/ThreadTest.cpp
	tests/TokenizerTest.cpp
	tests/TupleTest.cpp
	tests/VarTest.cpp
	tests/VectorTest.cpp
)

gtest_suite_begin(tests-${LIB} TEMPLATE ${ROOT_DIR}/src/modules/core/tests/main.cpp.in)
gtest_suite_sources(tests-${LIB} ${TEST_SRCS})
gtest_suite_deps(tests-${LIB} ${LIB})
gtest_suite_end(tests-${LIB})

set(BENCHMARK_SRCS
	benchmarks/CollectionBenchmark.cpp
)
engine_add_executable(TARGET benchmarks-${LIB} SRCS ${BENCHMARK_SRCS} NOINSTALL)
engine_target_link_libraries(TARGET benchmarks-${LIB} DEPENDENCIES benchmark-app)
