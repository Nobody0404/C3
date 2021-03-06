#pragma once

#include "Common/MWR/CppTools/ByteView.h"

namespace MWR
{
	using HashT = std::uint32_t;
}

namespace MWR::C3
{
	//Forward declaration
	class InterfaceFactory;

	/// Abstract Relay.
	struct Relay
	{
		/// Waits for Relay to be terminated internally by a C3 API Command (e.g. from WebController).
		virtual void Join() = 0;
	};

	/// An internal structure that represents a Log entry.
	struct LogMessage
	{
		/// Enumeration type for information severity.
		enum class Severity { Information, Warning, Error, DebugInformation };

		MWR::SecureString m_Body;																								///< Message's text.
		Severity m_Severity;																							///< Message's type.

		LogMessage(std::string_view message, Severity severity)
			: m_Body{ message }
			, m_Severity{ severity }
		{
		}
	};

	namespace Utils
	{
		/// Gate/Node Relay starters logger callback prototype.
		using LoggerCallback = void(*)(LogMessage const&, std::string_view*);

		/// Creates a Gateway.
		/// @param callbackOnLog callback fired whenever a new Log entry is being added.
		/// @keysFileName path to the encryption keys file.
		/// @configurationFileName path to the configuration file.
		/// @param interfaceFactory reference to interface factory.
		/// @return Shared Gateway object.
		std::shared_ptr<Relay> CreateGatewayFromConfigurationFiles(LoggerCallback callbackOnLog, InterfaceFactory& interfaceFactory, std::filesystem::path const& keysFileName, std::filesystem::path const& configurationFileName);

		/// Creates and starts a NodeRelay in a background thread.
		/// @param callbackOnLog callback fired whenever a new Log entry is being added.
		/// @param interfaceFactory reference to interface factory.
		std::shared_ptr<Relay> CreateNodeRelayFromImagePatch(LoggerCallback callbackOnLog, InterfaceFactory& interfaceFactory, ByteView buildId, ByteView gatewaySignature, ByteView broadcastKey, std::vector<ByteVector> const& gatewayInitialPackets);

		/// Converts Relay's Log entry into single line of text, ready to be printed e.g. on console screen.
		/// @param relayName name of the Relay.
		/// @param message information to log.
		/// @param sender ID of the Interface reporting the message. If sender.empty() then the message comes from internal Relay mechanisms. If sender is null then it comes from outside the Relay.
		/// @return Constructed string.
		std::string ConvertLogMessageToConsoleText(std::string_view relayName, MWR::C3::LogMessage const& message, std::string_view* sender);
	}

	/// Device bridge between C3 Core and C3 Mantle.
	struct AbstractDeviceBridge
	{
		/// Called by Relay just after the Device creation.
		virtual void OnAttach() = 0;

		/// Detaches the Device.
		virtual void Detach() = 0;

		/// Callback periodically fired by Relay for Device to update it's state. Might be called from a separate thread. Device should perform all necessary actions and leave as soon as possible.
		virtual void OnReceive() = 0;

		/// Fired by Channel when a C3 packet arrives.
		/// @param packet full C3 packet.
		virtual void PassNetworkPacket(ByteView packet) = 0;

		/// Fired by Relay to pass provided C3 packet through the Channel Device.
		/// @param packet full C3 packet.
		virtual void OnPassNetworkPacket(ByteView packet) = 0;

		/// Called whenever Peripheral wants to send a Command to its Connector Binder.
		/// @param command full Command with arguments.
		virtual void PostCommandToConnector(ByteView command) = 0;

		/// Fired by Relay to pass by provided Command from Connector.
		/// @param command full Command with arguments.
		virtual void OnCommandFromConnector(ByteView command) = 0;

		/// Runs Device's Command.
		/// @param command Device's Command to run.
		/// @return Command result.
		virtual ByteVector RunCommand(ByteView command) = 0;

		/// Tells Device's type name.
		/// @return A buffer with Device's description.
		virtual ByteVector WhoAreYou() = 0;

		/// Logs a message. Used by internal mechanisms to report errors, warnings, informations and debug messages.
		/// @param message information to log.
		virtual void Log(LogMessage const& message) = 0;

		/// Modifies the duration and jitter of OnReceive() calls. If minUpdateFrequencyInMs != maxUpdateFrequencyInMs then update frequency is randomized in range between those values.
		/// @param minUpdateFrequencyInMs minimum update frequency.
		/// @param maxUpdateFrequencyInMs maximum update frequency.
		virtual void SetUpdateFrequency(std::chrono::milliseconds minUpdateFrequencyInMs, std::chrono::milliseconds maxUpdateFrequencyInMs) = 0;

		/// Sets time span between OnReceive() calls to a fixed value.
		/// @param frequencyInMs frequency of OnReceive() calls.
		virtual void SetUpdateFrequency(std::chrono::milliseconds frequencyInMs) = 0;

		virtual void SetErrorStatus(std::string_view errorMessage) = 0;
		virtual std::string GetErrorStatus() = 0;
	};

 	/// Connector bridge between C3 Core and C3 Mantle.
 	struct AbstractConnectorBridge
	{
		/// Called by Relay just after creation of the Connector.
		virtual void OnAttach() = 0;

		/// Detaches the Connector.
		virtual void Detach() = 0;

		/// Called whenever Connector wants to send a Command to its Peripheral Binder.
		/// @param binderId Identifier of Peripheral who sends the Command.
		/// @param command full Command with arguments.
		virtual void PostCommandToBinder(ByteView binderId, ByteView command) = 0;

		/// Fired by Relay to pass by provided Command from Binder Peripheral.
		/// @param binderId Identifier of Peripheral who sends the Command.
		/// @param command full Command with arguments.
		virtual void OnCommandFromBinder(ByteView binderId, ByteView command) = 0;

		/// Runs Connector's Command.
		/// @param command Connector's Command to run.
		/// @return Command result.
		virtual ByteVector RunCommand(ByteView command) = 0;

		virtual ByteVector PeripheralCreationCommand(ByteView connectionId, ByteView data, bool isX64 = false) = 0;

		/// Logs a message. Used by internal mechanisms to report errors, warnings, informations and debug messages.
		/// @param message information to log.
		virtual void Log(LogMessage const& message) = 0;

		virtual bool IsAlive() const = 0;

		virtual void SetErrorStatus(std::string_view errorMessage) = 0;
		virtual std::string GetErrorStatus() = 0;
	};
}
