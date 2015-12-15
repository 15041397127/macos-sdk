/*
 * Copyright (c) 1998-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __OPEN_SOURCE__
/*
 *
 *	$Log: IOUSBCommand.h,v $
 *	Revision 1.17  2005/12/07 21:53:32  nano
 *	Bring in fixes from branch PR-4304258 -- Low Latency Audio support in  Yellow
 *	
 *	Revision 1.16.70.1  2005/12/02 20:25:32  nano
 *	Add a field indicating whether a command was created due to the client being a Rosetta app
 *	
 *	Revision 1.16  2005/07/06 23:13:54  rhoads
 *	roll changes from the TigerUpdate branch into TOT in preparation for the big Karma merge
 *	
 *	Revision 1.15.100.1  2005/07/06 05:37:37  nano
 *	Fixed bug where we were re-using IOUSBCommands
 *	
 *	Revision 1.15.144.1  2005/07/01 19:20:47  nano
 *	Fix for <rdar://problem/4130697> IOUSBCommand seems to get freed twice w/out some debugging code in the non-2 branch
 *	
 *	Revision 1.15.126.1  2005/05/27 18:42:48  nano
 *	Add a field to the IOUSBCommand that indicates whether the transfer is sync or async.
 *	
 *	Revision 1.15  2004/05/17 21:52:50  nano
 *	Add timeStamp and useTimeStamp to our commands.
 *	
 *	Revision 1.14.16.1  2004/05/17 15:57:27  nano
 *	API Changes for Tiger
 *	
 *	Revision 1.14  2004/02/03 22:09:49  nano
 *	Fix <rdar://problem/3548194>: Remove $ Id $ from source files to prevent conflicts
 *	
 *	Revision 1.13.2.2  2004/04/28 17:26:09  nano
 *	Remove $ ID $ so that we don't get conflicts on merge
 *	
 *	Revision 1.13.2.1  2003/11/04 22:27:37  nano
 *	Work in progress to add time stamping to interrupt handler
 *	
 *	Revision 1.13  2003/10/14 22:05:06  nano
 *	Add a GetStatus() method.
 *	
 *	Revision 1.12.40.1  2003/09/25 19:58:21  nano
 *	Add GetStatus to isoc command
 *	
 *	Revision 1.12  2003/08/20 19:41:40  nano
 *	
 *	Bug #:
 *	New version's of Nima's USB Prober (2.2b17)
 *	3382540  Panther: Ejecting a USB CardBus card can freeze a machine
 *	3358482  Device Busy message with Modems and IOUSBFamily 201.2.14 after sleep
 *	3385948  Need to implement device recovery on High Speed Transaction errors to full speed devices
 *	3377037  USB EHCI: returnTransactions can cause unstable queue if transactions are aborted
 *	
 *	Also, updated most files to use the id/log functions of cvs
 *	
 *	Submitted by: nano
 *	Reviewed by: rhoads/barryt/nano
 *	
 */
#endif
#ifndef _IOKIT_IOUSBCOMMAND_H
#define _IOKIT_IOUSBCOMMAND_H

#include <IOKit/IOCommand.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/usb/USB.h>

/*
 * USB Command
 * This is the command block for a USB command to be queued on the
 * Command Gate of the WorkLoop.  Although the actual work of creating
 * the command and enqueueing it is done for them via the deviceRequest,
 * read, and write methods, this is the primary way that devices will get
 * work done.
 * Note: This is private to IOUSBController and should be moved to a
 * private header.
 */

typedef enum {
    DEVICE_REQUEST,	// Device request using pointer
    READ,
    WRITE,
    CREATE_EP,
    DELETE_EP,
    DEVICE_REQUEST_DESC	// Device request using descriptor
} usbCommand;



class IOUSBCommand : public IOCommand
{
    OSDeclareAbstractStructors(IOUSBCommand)

protected:
    usbCommand				_selector;
    IOUSBDeviceRequestPtr	_request;
    USBDeviceAddress		_address;
    UInt8					_endpoint;
    UInt8					_direction;
    UInt8					_type;
    bool					_bufferRounding;
    IOMemoryDescriptor *	_buffer;
    IOUSBCompletion			_uslCompletion;
    IOUSBCompletion			_clientCompletion;
    UInt32					_dataRemaining;							// For Control transfers
    UInt8					_stage;									// For Control transfers
    IOReturn				_status;
    IOMemoryDescriptor *	_origBuffer;
    IOUSBCompletion			_disjointCompletion;
    IOByteCount				_dblBufLength;
    UInt32					_noDataTimeout;
    UInt32					_completionTimeout;
    UInt32					_UIMScratch[10];
    
    struct ExpansionData
    {
        IOByteCount			_reqCount;
		IOMemoryDescriptor *_requestMemoryDescriptor;
		IOMemoryDescriptor *_bufferMemoryDescriptor;
		bool				_multiTransferTransaction;
		bool				_finalTransferInTransaction;
        bool				_useTimeStamp;
        AbsoluteTime		_timeStamp;
		bool				_isSyncTransfer;						// Returns true if the command is used for a synchronous transfer
    };
    ExpansionData * 		_expansionData;
    
    // we override these OSObject method in order to allocate and release our expansion data
    virtual bool init();
    virtual void free();

public:

    // static constructor
    static IOUSBCommand *	NewCommand(void);

    // Manipulators
    void					SetSelector(usbCommand sel);
    void  					SetRequest(IOUSBDeviceRequestPtr req);
    void  					SetAddress(USBDeviceAddress addr);
    void  					SetEndpoint(UInt8 ep);
    void  					SetDirection(UInt8 dir);
    void  					SetType(UInt8 type);
    void  					SetBufferRounding(bool br);
    void  					SetBuffer(IOMemoryDescriptor *buf);
    void  					SetUSLCompletion(IOUSBCompletion completion);
    void  					SetClientCompletion(IOUSBCompletion completion);
    void  					SetDataRemaining(UInt32 dr);
    void  					SetStage(UInt8 stage);
    void 					SetStatus(IOReturn stat);
    void 					SetOrigBuffer(IOMemoryDescriptor *buf);
    void 					SetDisjointCompletion(IOUSBCompletion completion);
    void 					SetDblBufLength(IOByteCount len);
    void 					SetNoDataTimeout(UInt32 to);
    void 					SetCompletionTimeout(UInt32 to);
    void 					SetUIMScratch(UInt32 index, UInt32 value);
    void 					SetReqCount(IOByteCount reqCount);
    void					SetRequestMemoryDescriptor(IOMemoryDescriptor *requestMemoryDescriptor);
    void					SetBufferMemoryDescriptor(IOMemoryDescriptor *bufferMemoryDescriptor);
    void					SetMultiTransferTransaction(bool);
    void					SetFinalTransferInTransaction(bool);
    void					SetUseTimeStamp(bool);
    void					SetTimeStamp(AbsoluteTime timeStamp);
	void					SetIsSyncTransfer(bool);
	
	// Accessors
    usbCommand				GetSelector(void);
    IOUSBDeviceRequestPtr 	GetRequest(void);
    USBDeviceAddress 		GetAddress(void);
    UInt8					GetEndpoint(void);
    UInt8					GetDirection(void);
    UInt8					GetType(void);
    bool					GetBufferRounding(void);
    IOMemoryDescriptor * 	GetBuffer(void);
    IOUSBCompletion 		GetUSLCompletion(void);
    IOUSBCompletion 		GetClientCompletion(void);
    UInt32					GetDataRemaining(void);
    UInt8					GetStage(void);
    IOReturn				GetStatus(void);
    IOMemoryDescriptor *	GetOrigBuffer(void);
    IOUSBCompletion 		GetDisjointCompletion(void);
    IOByteCount				GetDblBufLength(void);
    UInt32					GetNoDataTimeout(void);
    UInt32					GetCompletionTimeout(void);
    UInt32					GetUIMScratch(UInt32 index);
    IOByteCount				GetReqCount(void);
    IOMemoryDescriptor *	GetRequestMemoryDescriptor(void);
    IOMemoryDescriptor *	GetBufferMemoryDescriptor(void);
    bool					GetMultiTransferTransaction(void);
    bool					GetFinalTransferInTransaction(void);
    bool					GetUseTimeStamp(void);
    AbsoluteTime			GetTimeStamp(void);
	bool					GetIsSyncTransfer(void);
};


class IOUSBIsocCommand : public IOCommand
{
    OSDeclareAbstractStructors(IOUSBIsocCommand)

protected:
    usbCommand				_selector;
    USBDeviceAddress		_address;
    UInt8					_endpoint;
    UInt8					_direction;
    IOMemoryDescriptor *	_buffer;
    IOUSBIsocCompletion		_completion;
    UInt64					_startFrame;
    UInt32					_numFrames;
    IOUSBIsocFrame *		_frameList;
    IOReturn				_status;
    
    struct ExpansionData
    {
        UInt32				_updateFrequency;
        bool				_useTimeStamp;
        AbsoluteTime		_timeStamp;
		bool				_isSyncTransfer;								// Returns true if the command is used for a synchronous transfer
		bool				_rosettaClient;
    };
    ExpansionData * 		_expansionData;

    // we override these OSObject method in order to allocate and release our expansion data
    virtual bool init();
    virtual void free();

public:
    // static constructor
    static IOUSBIsocCommand *NewCommand(void);
    
    // Manipulators
    void					SetSelector(usbCommand sel)						{_selector = sel; }
    void					SetAddress(USBDeviceAddress addr)				{_address = addr; }
    void					SetEndpoint(UInt8 ep)							{_endpoint = ep; }
    void					SetDirection(UInt8 dir)							{_direction = dir; }
    void					SetBuffer(IOMemoryDescriptor *buf)				{_buffer = buf; }
    void					SetCompletion(IOUSBIsocCompletion completion)	{_completion = completion; }
    void					SetStartFrame(UInt64 sf)						{_startFrame = sf; }
    void					SetNumFrames(UInt32 nf)							{_numFrames = nf; }
    void					SetFrameList(IOUSBIsocFrame *fl)				{_frameList = fl; }
    void					SetStatus(IOReturn stat)						{_status = stat; }
    void					SetUpdateFrequency( UInt32 fr)					{ _expansionData->_updateFrequency = fr; }
    void					SetUseTimeStamp(bool useIt)						{ _expansionData->_useTimeStamp= useIt; }
    void					SetTimeStamp(AbsoluteTime timeStamp)			{ _expansionData->_timeStamp= timeStamp; }
 	void					SetIsSyncTransfer(bool isSync)					{ _expansionData->_isSyncTransfer = isSync; }
 	void					SetRosettaClient(bool isRosetta)				{ _expansionData->_rosettaClient = isRosetta; }
   
	// Accessors
    usbCommand				GetSelector(void)								{ return _selector; }
    USBDeviceAddress		GetAddress(void)								{ return _address; }
    UInt8					GetEndpoint(void)								{ return _endpoint; }
    UInt8					GetDirection(void)								{ return _direction; }
    IOMemoryDescriptor *	GetBuffer(void)									{ return _buffer; }
    IOUSBIsocCompletion		GetCompletion(void)								{ return _completion; }
    UInt64					GetStartFrame(void)								{ return _startFrame; }
    UInt32					GetNumFrames(void)								{ return _numFrames; }
    IOUSBIsocFrame *		GetFrameList(void)								{ return _frameList; }
    UInt32					GetUpdateFrequency(void)						{ return _expansionData->_updateFrequency; }
    IOReturn				GetStatus(void)									{ return _status; }
    bool					GetUseTimeStamp(void)							{ return _expansionData->_useTimeStamp; }
    AbsoluteTime			GetTimeStamp(void)								{ return _expansionData->_timeStamp; }
	bool					GetIsSyncTransfer(void)							{ return _expansionData->_isSyncTransfer; }
	bool					GetIsRosettaClient(void)							{ return _expansionData->_rosettaClient; }
};


#endif
