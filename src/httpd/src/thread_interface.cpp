#include "http.h"

initialiseSingleton( Httpd );

Httpd::Httpd()
{
}

Httpd::~Httpd()
{
}

extern "C"
{

#include "main.h"

void Httpd::SetInitialHttpdSettings()
{
	pthread_attr_t thr_attr;

	init();

	if (pthread_attr_init(&thr_attr)) {
		logerror("pthread_attr_init()");
		exit(1);
	}
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) {
		logerror("pthread_attr_setstacksize()");
		exit(1);
	}
}

extern void internal_accept_loop(void *x);

void Httpd::Update(time_t diff)
{
	internal_accept_loop(NULL);
}

}
