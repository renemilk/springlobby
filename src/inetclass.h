/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef SPRINGLOBBY_HEADERGUARD_INETCLASS_H
#define SPRINGLOBBY_HEADERGUARD_INETCLASS_H

class Socket;

//! @brief Abstract baseclass that is used when needed to interface with socket class
class iNetClass
{
public:

	iNetClass() {}
	virtual ~iNetClass() {}

	virtual void OnConnected( Socket& /*sock*/ ) {}
	virtual void OnDisconnected( Socket& /*sock*/ ) {}
	virtual void OnDataReceived( Socket& /*sock*/ ) {}
	virtual void Ping() {}
};

class FakeNetClass : public iNetClass
{
public:

	FakeNetClass() {}
	~FakeNetClass() {}

	void OnConnected( Socket& /*unused*/ ) {}
	void OnDisconnected( Socket& /*unused*/ ) {}
	void OnDataReceived( Socket& /*unused*/ ) {}
	void Ping() {}
};

#endif // SPRINGLOBBY_HEADERGUARD_INETCLASS_H

