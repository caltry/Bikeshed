#ifndef __CPP_TEST_H__
#define __CPP_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

void _test_cpp(void);

#ifdef __cplusplus
}

class Object
{
public:
	Object(int i);
	~Object();

	void increment();
	int get_i();

private:
	int _i;
};
#endif


#endif
