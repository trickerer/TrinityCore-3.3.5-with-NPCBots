/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TCSoap.h"
#include "soapH.h"
#include "soapStub.h"
#include "Realm.h"
#include "World.h"
#include "AccountMgr.h"
#include "Log.h"

#include "ChatCommand.h"  // Include the header for SendWorldMessageCommand
#include <string>

// SOAP Handler Class
class SOAPHandler {
public:
    bool SOAPHandler::HandleSendWorldMessage(const std::string& message)
	{
		// Create an instance of the fully qualified SendWorldMessageCommand
		Trinity::ChatCommands::SendWorldMessageCommand cmd;

		// Call the HandleCommand method and store the result in 'success'
		bool success = cmd.HandleCommand(nullptr, message);  // Passing nullptr for session as we don’t have one

		if (success)
		{
			// Successfully sent the message, return 0 or some success code
			return 0;
		}
		else
		{
			// If the message was not successfully sent, return an error code
			return -1;
		}
	}

};

// Main SOAP Thread
void TCSoapThread(const std::string& host, uint16 port)
{
    struct soap soap;
    soap_init(&soap);
    soap_set_imode(&soap, SOAP_C_UTFSTRING);
    soap_set_omode(&soap, SOAP_C_UTFSTRING);

#if TRINITY_PLATFORM != TRINITY_PLATFORM_WINDOWS
    soap.bind_flags = SO_REUSEADDR;
#endif

    // Check every 3 seconds if world ended
    soap.accept_timeout = 3;
    soap.recv_timeout = 5;
    soap.send_timeout = 5;
    if (!soap_valid_socket(soap_bind(&soap, host.c_str(), port, 100)))
    {
        TC_LOG_ERROR("network.soap", "Couldn't bind to {}:{}", host, port);
        exit(-1);
    }

    TC_LOG_INFO("network.soap", "Bound to http://{}:{}", host, port);

    while (!World::IsStopped())
    {
        if (!soap_valid_socket(soap_accept(&soap)))
            continue;   // Ran into an accept timeout

        TC_LOG_DEBUG("network.soap", "Accepted connection from IP={}.{}.{}.{}", (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
        struct soap* thread_soap = soap_copy(&soap);  // Make a safe copy
        process_message(thread_soap);
    }

    soap_destroy(&soap);
    soap_end(&soap);
    soap_done(&soap);
}

// Process SOAP Message
void process_message(struct soap* soap_message)
{
    TC_LOG_TRACE("network.soap", "SOAPWorkingThread::process_message");

    soap_serve(soap_message);
    soap_destroy(soap_message); // Dealloc C++ data
    soap_end(soap_message); // Dealloc data and clean up
    soap_free(soap_message); // Detach soap struct and free up the memory
}

// SOAP Command Execution
int ns1__executeCommand(soap* soap, char* command, char** result)
{
    if (!soap->userid || !soap->passwd)
    {
        TC_LOG_INFO("network.soap", "Client didn't provide login information");
        return 401;
    }

    uint32 accountId = AccountMgr::GetId(soap->userid);
    if (!accountId)
    {
        TC_LOG_INFO("network.soap", "Client used invalid username '{}'", soap->userid);
        return 401;
    }

    if (!AccountMgr::CheckPassword(accountId, soap->passwd))
    {
        TC_LOG_INFO("network.soap", "Invalid password for account '{}'", soap->userid);
        return 401;
    }

    if (AccountMgr::GetSecurity(accountId, realm.Id.Realm) < SEC_ADMINISTRATOR)
    {
        TC_LOG_INFO("network.soap", "{}'s gmlevel is too low", soap->userid);
        return 403;
    }

    if (!command || !*command)
        return soap_sender_fault(soap, "Command can not be empty", "The supplied command was an empty string");

    TC_LOG_INFO("network.soap", "Received command '{}'", command);

    // Convert char* to std::string before passing to HandleSendWorldMessage
    std::string commandStr(command);

    SOAPHandler handler;
    bool SOAPHandler::HandleSendWorldMessage(const std::string& message)
	{
		SendWorldMessageCommand cmd;  // Create an instance of the SendWorldMessageCommand
		bool success = cmd.HandleCommand(nullptr, message);  // Passing nullptr for session since we don't have one in this context
		return success;
	}

    if (success)
    {
        *result = soap_strdup(soap, "World message sent successfully");
        return SOAP_OK;
    }
    else
    {
        *result = soap_strdup(soap, "Failed to send world message");
        return soap_sender_fault(soap, "Failed to send message", "An error occurred while sending the world message");
    }
}

// Namespace Definitions for SOAP
struct Namespace namespaces[] =
{   
    { "SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", NULL, NULL }, // Must be first
    { "SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", NULL, NULL }, // Must be second
    { "xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL },
    { "xsd", "http://www.w3.org/1999/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL },
    { "ns1", "urn:TC", NULL, NULL },     // "ns1" namespace prefix
    { NULL, NULL, NULL, NULL }
};
