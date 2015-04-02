/**
 * @file   E_Module.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Module
 */

#ifndef E_MODULE_HPP_
#define E_MODULE_HPP_

#include <E/E_Common.hpp>

namespace E
{
class System;

/**
 * @brief Module is an interface for classes
 * which can send/receive Message to/from the System.
 *
 * @see System
 */
class Module
{
private:
	System* system;

public:
	/**
	 * @brief Module must belong to a System.
	 * Module is registered to a System when it is created.
	 * @param system System to be registered.
	 */
	Module(System* system);
	virtual ~Module();

	/**
	 * @return System this module is registered.
	 * @note You cannot override this function.
	 */
	virtual System* getSystem() final;

	/**
	 * @brief Interface of Message. Every message implementation
	 * must inherit this class.
	 *
	 * @see Module::messageReceived, Module::messageFinished, and Module::messageCancelled for further information.
	 */
	class Message
	{
	public:
		Message(){}
		virtual ~Message(){}
	};

protected:

	/**
	 * @brief This is a callback function called by the System.
	 * This function is automatically called when you received a Message.
	 * If you want to handle Message, override this function with your own handler.
	 *
	 * @param from Source module which created and sent the message.
	 * This cannot be null (someone must create the message).
	 * If you sent a message to yourself, this parameter would be [this].
	 * @param message Message you received. This message is allocated by other module, so
	 * DO NOT DEALLOCATE THE GIVEN MESSAGE (even it is your self message).
	 * @return If you want to give a feedback to the sender of this message,
	 * ALLOCATE ANOTHER MESSAGE and return it.
	 *
	 * @note Using default implementation is forbidden.
	 * If you use the default implementation, no module can send messages to you.
	 */
	virtual Module::Message* messageReceived(Module* from, Module::Message* message) {assert(0); return nullptr;}

	/**
	 * @brief This is a callback function called by the System.
	 * This function is automatically called after your message is processed by the destination module.
	 * If you want to handle Message, override this function with your own handler.
	 * YOU MUST DEALLOCATE EVERY SUCCESSFUL MESSAGE YOU ALLOCATED JUST HERE.
	 * INSTANT RESPONSE messages(return value of messageReceived) are also deallocated here.
	 * SELF messages are also deallocated here.
	 *
	 * @param to Destination of the Message(including [this]).
	 * If you sent a message to yourself, this parameter would be [this].
	 * @param message Message to be deallocated.
	 * This is the message you allocated.
	 * @param response If the message is successfully processed,
	 * the receiver may give an instance response by return value.
	 * Response message is deallocated by the responder's messageFinished, so
	 * DO NOT DEALLOCATE THE RESPONSE MESSAGE.
	 *
	 * @note Using default implementation is forbidden.
	 * If you use send any message, you must override this function with your own implementation.
	 */
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) {assert(0);}

	/**
	 * @brief This is a callback function called by the System.
	 * This function is automatically called when the message is cancelled before it is processed.
	 * This function is called AT THE TIME WHEN THE MESSAGE WAS SUPPOSED TO BE SENT.
	 * If you want to handle Message, override this function with your own handler.
	 * YOU MUST DEALLOCATE EVERY CANCELLED MESSAGE YOU ALLOCATED JUST HERE.
	 *
	 * @param to Destination of the Message(including [this]).
	 * If you sent a message to yourself, this parameter would be [this].
	 * @param message Message to be deallocated.
	 * This is the message you allocated.
	 *
	 * @note Using default implementation is forbidden.
	 * If you use send any message, you must override this function with your own implementation.
	 * This function is similar to messageFinished except its response message is always null.
	 *
	 * @see messageFinished
	 */
	virtual void messageCancelled(Module* to, Module::Message* message) {assert(0);}

	/**
	 * @brief Send a Message to other Module.
	 * Every message has its own delay before it is actually sent.
	 * The System guarantees the total ordering of all delayed messages in the System.
	 *
	 * @param to Destination Module. You can send a Message to yourself (this).
	 * @param message Message to be sent. YOU MUST ALLOCATE THE MESSAGE TO BE SENT.
	 * @param timeAfter Delay of this message. The receiver will receive the Message at [current time] + [delay].
	 * @return UUID of generated message. This UUID is used for cancellation of the message.
	 *
	 * @note You cannot override this function.
	 * @see messageFinished and messageReceived for allocate/deallocate convention.
	 */
	virtual UUID sendMessage(Module* to, Module::Message* message, Time timeAfter) final;

	/**
	 * @brief Cancel the raised Message.
	 * If a message is not actually sent yet, you can cancel the message.
	 * If the message is already sent, this function has no effect.
	 *
	 * @param messageID Unique ID that represents the target Message to be cancelled.
	 * @return Whether the cancellation is successful.
	 *
	 * @note You cannot override this function.
	 * @see sendMessage
	 */
	virtual bool cancelMessage(UUID messageID) final;

	friend class System;
};

}


#endif /* E_MODULE_HPP_ */
