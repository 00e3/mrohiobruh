#pragma once
#include <cstdint>

#define EVENT_DEBUG_ID_INIT 42 
#define EVENT_DEBUG_ID_SHUTDOWN 13  

class IGameEvent
{
public:
  virtual ~IGameEvent() = 0;
  virtual const char*     GetName() const = 0;

  virtual bool            IsReliable() const = 0;
  virtual bool            IsLocal() const = 0;
  virtual bool            IsEmpty(const char *keyName = nullptr) = 0;

  virtual bool            GetBool(const char *keyName = nullptr, bool defaultValue = false) = 0;
  virtual int             GetInt(const char *keyName = nullptr, int defaultValue = 0) = 0;
  virtual uint64_t        GetUint64(const char *keyName = nullptr, unsigned long defaultValue = 0) = 0;
  virtual float           GetFloat(const char *keyName = nullptr, float defaultValue = 0.0f) = 0;
  virtual const char*     GetString(const char *keyName = nullptr, const char *defaultValue = "") = 0;
  virtual const wchar_t*  GetWString(const char *keyName, const wchar_t *defaultValue = L"") = 0;
  virtual const void *	  GetPointer( const char *keyName = nullptr, const void *defaultValue = nullptr ) = 0;

  virtual void            SetBool(const char *keyName, bool value) = 0;
  virtual void            SetInt(const char *keyName, int value) = 0;
  virtual void            SetUint64(const char *keyName, unsigned long value) = 0;
  virtual void            SetFloat(const char *keyName, float value) = 0;
  virtual void            SetString(const char *keyName, const char *value) = 0;
  virtual void            SetWString(const char *keyName, const wchar_t *value) = 0;
  virtual void			  SetPointer( const char *keyName, const void *value ) = 0;

  bool get_bool( const char* keyName ) {
	  return GetBool( keyName );
  }

  int get_int( const char* keyName ) {
	  return GetInt( keyName );
  }

  uint64_t get_uint64( const char* keyName ) {
	  return GetUint64( keyName );
  }

  float get_float( const char* keyName ) {
	  return GetFloat( keyName );
  }

  const char* get_string( const char* keyName ) {
	  return GetString( keyName );
  }

  const wchar_t* get_wstring( const char* keyName ) {
	  return GetWString( keyName );
  }
};

class IGameEventListener
{
public:
  virtual ~IGameEventListener(void) {}

  virtual void FireGameEvent(IGameEvent *event) = 0;
  virtual int  GetEventDebugID(void) = 0;

public:
  int m_iDebugId;
};

class IGameEventManager
{
public:
  virtual             ~IGameEventManager() = 0;
  virtual int         LoadEventsFromFile(const char *filename) = 0;
  virtual void        Reset() = 0;
  virtual bool        AddListener(IGameEventListener *listener, const char *name, bool bServerSide) = 0;
  virtual bool        FindListener(IGameEventListener *listener, const char *name) = 0;
  virtual int         RemoveListener(IGameEventListener *listener) = 0;
  virtual IGameEvent* CreateEvent(const char *name, bool bForce, unsigned int dwUnknown) = 0;
  virtual bool        FireEvent(IGameEvent *event, bool bDontBroadcast = false) = 0;
  virtual bool        FireEventClientSide(IGameEvent *event) = 0;
  virtual IGameEvent* DuplicateEvent(IGameEvent *event) = 0;
  virtual void        FreeEvent(IGameEvent *event) = 0;
  virtual bool        SerializeEvent(IGameEvent *event, bf_write *buf) = 0;
  virtual IGameEvent* UnserializeEvent(bf_read *buf) = 0;
};
