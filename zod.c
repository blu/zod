#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LIST_DTOR_MAX_LEN 64

typedef struct {
	void *self;
	void (*dtor)(void*);
}  term_t;

void __attribute__ ((always_inline))
term_scope(term_t *restrict list_dtor, const size_t count) {
	term_t *ip = list_dtor + count - 1;
	for (size_t i = 0; i < count; --ip, ++i) {
		assert(ip->dtor);
		ip->dtor(ip->self);
	}
}

size_t __attribute__ ((always_inline))
insert_dtor(term_t *restrict list_dtor, const size_t count, void *self, void (*dtor)(void*)) {
	assert(self);
	assert(dtor);
	assert(count < LIST_DTOR_MAX_LEN);

	term_t *ip = list_dtor + count;
	ip->self = self;
	ip->dtor = dtor;
	return count + 1;
}

// macros begin_scope, end_scope and construct take a common argument 'level' that signifies
// the nesting depth of the scope where the macro is used; the top scope of a function is
// level 0; there is no limit on the deepest level

// rules of use: at scopes where ctors/dtors will be used, begin_scope is invoked with the
// respective nesting level as an argument; at all points of exit from such scopes either
// one or several end_scope are invoked, depending on how many scope levels this particular
// exit point spans, e.g. function-returning from a nested scope affects all parent scopes

#define begin_scope(level) \
	term_t list_dtor_ ## level[LIST_DTOR_MAX_LEN] = {}; \
	size_t list_dtor_ ## level ## _count = 0;

#define end_scope(level) \
	term_scope(list_dtor_ ## level, list_dtor_ ## level ## _count);

#define construct(level, var, ctor) \
	list_dtor_ ## level ## _count = ctor(&var, list_dtor_ ## level, list_dtor_ ## level ## _count);

// struct Foo employing ctor/dtor
typedef struct {
	int val;
} Foo;

void __attribute__ ((noinline))
Foo_dtor(Foo* self) {
	assert(self);

	// deinit some
	self->val = 43;
	fprintf(stderr, "%s, %p, val: %d\n", __FUNCTION__, self, self->val);
}

size_t __attribute__ ((always_inline))
Foo_ctor(Foo* self, term_t *restrict list_dtor, const size_t count) {
	assert(self);
	assert(list_dtor);

	// init some
	self->val = 42;
	fprintf(stderr, "%s, %p, val: %d\n", __FUNCTION__, self, self->val);

	// ctor concludes with registering the dtor closure into the supplied array
	return insert_dtor(list_dtor, count, self, Foo_dtor);
}

int main(int argc, char** argv) {
	begin_scope(0);

	Foo f;
	construct(0, f, Foo_ctor);

	if (argc > 1) {
		begin_scope(1);

		Foo g;
		construct(1, g, Foo_ctor);
		Foo h;
		construct(1, h, Foo_ctor);

		if (atoi(argv[1]) == 42) {
			end_scope(1)
			end_scope(0)
			return EXIT_FAILURE;
		}

		end_scope(1);
	}

	Foo g;
	construct(0, g, Foo_ctor);

	end_scope(0);
	return EXIT_SUCCESS;
}
