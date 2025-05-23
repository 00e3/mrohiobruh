#include "threading.h"
#include <thread>
#include "../Math.hpp"
#include "../extern/syscall.hpp"
static LList<struct Job> jobs;

uint64_t Threading::_QueueJob( JobFn function, void* data, bool ref, bool priority ) {
   Job job;
   job.args = data;
   job.function = function;
   job.ref = ref;
   uint64_t ret = jobs.Enqueue( job, priority );
   return ret;
}

static void RunJob( struct Job& job ) {
   job.function( job.args );
   if ( !job.ref )
	  free( job.args );
}

static void* __stdcall ThreadLoop( void* t ) {
   struct JobThread* thread = ( struct JobThread* )t;

   struct Job job;
   thread->isRunning = true;
   while ( !thread->shouldQuit ) {
	   while( !g_initialised && Threading::bDone ) {
		   std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	   }

	  if ( job.id ^ ~0ull ) {
		 thread->queueEmpty = false;
		 RunJob( job );

		// printf( "ran job in thread %i\n", thread->id );
	  } else
		 thread->queueEmpty = true;
	  struct LList<struct Job>* tJobs = thread->jobs;
	  thread->jLock->unlock( );
	  job = tJobs->PopFront( thread->jLock );
   }
   thread->isRunning = false;
   return nullptr;
}

unsigned int Threading::numThreads = 0;
static struct JobThread* threads = nullptr;

static void InitThread( struct JobThread* thread, int id ) {
   thread->id = id;
   thread->jLock = new Mutex( );
   thread->jobs = &jobs;
   thread->handle = Threading::StartThread( ThreadLoop, thread, false );
}

void Threading::InitThreads( ) {
   numThreads = std::thread::hardware_concurrency( );
   numThreads = Math::Clamp<unsigned int>( numThreads, 2, 12 );

   // apparently some peoples cpus are actually trash as fuck
   // so we gotta reduce this by 3
   if( numThreads >= 8 ) { // this should apply for any current 4+ core cpu
	  // ++numThreads;
   }
   // lol even worse cpu found (2 core?)
   else { 
	   // IF CRASH ; REVERRT
	  // if( numThreads <= 4 ) 
		//   numThreads = 2; // ragna moment
   }

   numThreads -= 2;
   numThreads = Math::Clamp<unsigned int>( numThreads, 2, 12 );

   // this guy...
   if( get_string_from_array( g_Vars.globals.user_info.the_array ) == XorStr( "Aszab123" ) ) {
	   numThreads = 6;
   }

   threads = ( struct JobThread* )calloc( numThreads, sizeof( struct JobThread ) );

   for ( unsigned int i = 0; i < numThreads; i++ )
	  InitThread( threads + i, i );

#ifdef DEV
   printf( XorStr( "%i threads initialized\n" ), numThreads );
#endif

   g_Vars.globals.m_nNumThreads = numThreads;
}

int Threading::EndThreads( ) {
   int ret = 0;

   if ( !threads )
	  return ret;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].shouldQuit = true;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].jobs->quit = true;

   for ( int o = 0; o < 4; o++ )
	  for ( unsigned int i = 0; i < numThreads; i++ )
		 threads[ i ].jobs->sem.Post( );

   for ( size_t i = 0; i < numThreads; i++ ) {
   #if defined(__linux__) || defined(__APPLE__)
	  void* ret2 = nullptr;
	  pthread_join( *( pthread_t* ) threads[ i ].handle, &ret2 );
   #else
	  ResumeThread( threads[ i ].handle );
	  if ( WaitForSingleObject( threads[ i ].handle, 100 ) == WAIT_TIMEOUT && threads[ i ].isRunning )
		 ;
   #endif
	  delete threads[ i ].jLock;
	  threads[ i ].jLock = nullptr;
	  CloseHandle( threads[ i ].handle );
   }
   free( threads );
   threads = nullptr;

   return ret;
}

void Threading::FinishQueue( bool executeJobs ) {
   if ( !threads )
	  return;

   if ( executeJobs ) {
	  for ( unsigned int i = 0; i < numThreads; i++ ) {
		 auto jobList = &jobs;
		 if ( threads[ i ].jobs )
			jobList = threads[ i ].jobs;
		 while ( 1 ) {
			struct Job job = jobList->TryPopFront( );
			if ( job.id == ~0ull )
			   break;
			RunJob( job );
		 }
	  }
   }

   if( g_Vars.globals.hackUnload || !threads )
	   return;

   for ( unsigned int i = 0; i < numThreads; i++ ) {
	   if( !threads )
		   break;

	  if ( threads[ i ].jobs )
		 while ( !threads[ i ].jobs->IsEmpty( ) );

	  threads[ i ].jLock->lock( );
	  threads[ i ].jLock->unlock( );
   }
}

JobThread* Threading::BindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  if ( threads[ i ].jobs == &jobs || !threads[ i ].jobs ) {
		 threads[ i ].jobs = jobsQueue;
		 for ( size_t o = 0; o < numThreads; o++ )
			jobs.sem.Post( );
		 return threads + i;
	  }
   }
   return nullptr;
}

void Threading::UnbindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  threads[ i ].jLock->lock( );
	  if ( threads[ i ].jobs == jobsQueue )
		 threads[ i ].jobs = &jobs;
	  threads[ i ].jLock->unlock( );
   }
}

thread_t Threading::StartThread( threadFn start, void* arg, bool detached ) {
#ifdef _WIN32
	thread_t thread;

	/*syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
		nullptr, arg, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	syscall( NtGetContextThread )( thread, &context );
	context.Eax = reinterpret_cast< uint32_t >( start );
	syscall( NtSetContextThread )( thread, &context );
	syscall( NtResumeThread )( thread, nullptr );*/

	//using CreateSimpleThread_t = thread_t( __cdecl * )( threadFn, void *, SIZE_T );
	//static auto CreateSimpleThread = reinterpret_cast< CreateSimpleThread_t >( LI_FN( GetProcAddress )( LI_FN( GetModuleHandleA )( XorStr( "tier0.dll" ) ), XorStr( "CreateSimpleThread" ) ) );
	// thread = CreateSimpleThread( start, arg, 0 );
	
	thread = CreateThread( nullptr, NULL, LPTHREAD_START_ROUTINE( start ), arg, NULL, nullptr );
	SetThreadPriority( thread, THREAD_PRIORITY_TIME_CRITICAL );
#else
	pthread_attr_t* attr = nullptr;
	pthread_attr_t tAttr;
	if( detached ) {
		pthread_attr_init( &tAttr );
		pthread_attr_setdetachstate( &tAttr, PTHREAD_CREATE_DETACHED );
		attr = &tAttr;
	}
	pthread_create( thread, attr, start, arg );
#endif
	return thread;
}


void Threading::JoinThread( thread_t thread, void** returnVal ) {
#ifdef __posix__
   pthread_join( thread, returnVal );
#else
   WaitForSingleObject( thread, INFINITE );
#endif
}

