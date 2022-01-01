//
//  CCServerManager.swift
//  chickencoop
//
//  Created by Vincent Moscaritolo on 1/1/22.
//

import Foundation
import SwiftRadix
import CoreLocation
import UIKit

 
enum  device_state_t :Int{
	case
		DEVICE_STATE_UNKNOWN = 0,
		DEVICE_STATE_DISCONNECTED,
		DEVICE_STATE_CONNECTED,
		DEVICE_STATE_ERROR,
		DEVICE_STATE_TIMEOUT
 }
 
extension device_state_t: Codable {
	
	enum Key: CodingKey {
		case rawValue
	}
	
	enum CodingError: Error {
		case unknownValue
	}
	
	init(from decoder: Decoder) throws {
		let container = try decoder.container(keyedBy: Key.self)
		let rawValue = try container.decode(Int.self, forKey: .rawValue)
		switch rawValue {
		case 0:
			self = .DEVICE_STATE_UNKNOWN
		case 1:
			self = .DEVICE_STATE_DISCONNECTED
		case 2:
			self = .DEVICE_STATE_CONNECTED
		case 3:
			self = .DEVICE_STATE_ERROR
		case 4:
			self = .DEVICE_STATE_TIMEOUT
	
		default:
			self = .DEVICE_STATE_UNKNOWN
		}
	}
	
	func encode(to encoder: Encoder) throws {
		var container = encoder.container(keyedBy: Key.self)
		switch self {
		case .DEVICE_STATE_UNKNOWN:
			try container.encode(0, forKey: .rawValue)
		case .DEVICE_STATE_DISCONNECTED:
			try container.encode(1, forKey: .rawValue)
		case .DEVICE_STATE_CONNECTED:
			try container.encode(2, forKey: .rawValue)
		case .DEVICE_STATE_ERROR:
			try container.encode(3, forKey: .rawValue)
		case .DEVICE_STATE_TIMEOUT:
			try container.encode(4, forKey: .rawValue)
		}
	}
}

class RESTErrorInfo: Codable {
	let code: Int
	let message: String
	let detail: String?
	let cmdDetail: String?
}

class RESTError: Codable {
	let error: RESTErrorInfo
}
 
enum RESTdeviceStatus : Int {
	case unknown = 0
	case disconnected
	case connected
	case error
	case timeout
}


struct RESTStatus: Codable {
	var state: Int
	var cpuTemp: Double?
	var stateString: String?
	var buildtime: String
	var version: String
	let date: String
	let uptime: Int
	let os_sysname : String
	let os_nodename : String
	let os_release : String
	let os_version : String
	let os_machine : String

	enum CodingKeys: String, CodingKey {
		case state   		= "state"
		case cpuTemp  		= "cpuTemp"
		case stateString  	= "stateString"
		case buildtime 		= "buildtime"
		case version 		= "version"
		case date 				= "date"
		case uptime 			= "uptime"
		case os_sysname 		= "os.sysname"
		case os_nodename 	= "os.nodename"
		case os_release 		= "os.release"
		case os_version 		= "os.version"
		case os_machine 		= "os.machine"
	}

}

class RESTDateInfo: Codable {
	let civilSunRise: Double?
	let civilSunSet: Double?
	let sunRise: Double?
	let sunSet: Double?
	let date: String
	let gmtOffset: Int?
	let latitude: Double?
	let longitude: Double?
	let midnight: Double?
	let timeZone: String?
	let uptime: Int
}


struct RESTValueDetails: Codable {
	var display: String
	var time:  Double
	var value: String
	
	enum CodingKeys: String, CodingKey {
		case display = "display"
		case time = "time"
		case value = "value"
	}
}
 
enum RESTschemaTracking : Int {
	case ignoreValue = 0
	case staticValue = 1
	case trackedValue = 2
	case untrackedValue = 3
}
 
enum RESTschemaUnits : Int {
	case INVALID = 0
	case BOOL				// Bool ON/OFF
	case INT			// Int
	case MAH				// mAh milliAmp hours
	case PERMILLE		// (per thousand) sign ‰
	case PERCENT		// (per hundred) sign ‰
	case DEKAWATTHOUR	 // .01kWh
	case WATTS			// W
	case MILLIVOLTS		// mV
	case MILLIAMPS		// mA
	case SECONDS			// sec
	case MINUTES			// mins
	case DEGREES_C		// degC
	case VOLTS			// V
	case HERTZ			// Hz
	case AMPS				// A
	case BINARY			// Binary 8 bits 000001
	case VE_PRODUCT		// VE.PART
	case STRING			// string
	case IGNORE
	case UNKNOWN
}

struct RESTSchemaDetails: Codable {
	var name: String
	var suffix:  String
	var tracking: Int
	var units: Int

	enum CodingKeys: String, CodingKey {
		case name = "name"
		case suffix = "suffix"
		case tracking = "tracking"
		case units = "units"
	}
	init() {
		name = String()
		suffix = String()
		tracking = RESTschemaTracking.ignoreValue.rawValue
		units = RESTschemaUnits.UNKNOWN.rawValue
	}
}

struct RESTSchemaList: Codable {
	var schema:  Dictionary<String, RESTSchemaDetails>

	enum CodingKeys: String, CodingKey {
		case schema = "schema"
	}
}


struct RESTProperties: Codable {
	var properties:  Dictionary<String, String>
  }

struct RESTValuesList: Codable {
	var values:  Dictionary<String, RESTValueDetails>
	var ETag: 	Int?
	
	var inverter: Int
	let inverterLastTime : Int
	var battery: Int
	let batteryLastTime : Int

	
	enum CodingKeys: String, CodingKey {
		case values = "values"
		case ETag = "ETag"
		case inverter 		= "inverter"
		case battery 		= "battery"
		case inverterLastTime 		= "inverter.lastTime"
		case batteryLastTime 		= "battery.lastTime"
	}
  }

struct RESTTimeSpanItem: Codable {
	var time:  		Double
	var durration:  TimeInterval
	var value: 		String
}

//struct RESTHistoryItem: Codable {
//	var time:  Double
//	var value: String
//
//	enum CodingKeys: String, CodingKey {
//		case time = "time"
//		case value = "value"
//	}
//}
//
//struct RESTHistory: Codable {
//	var values:  Array< RESTHistoryItem>
//	enum CodingKeys: String, CodingKey {
//		case values = "values"
//	}
//
//	func timeLine() -> Array<RESTTimeSpanItem> {
//
//		var timeline:Array<RESTTimeSpanItem> = []
//
//		let items = self.values.reversed()
//		let now = Date().timeIntervalSince1970
//		var lastValue:String = "<no value>"
//		var lastTime:Double = 0;
//
//		for item in items {
//
//			// did we change values
//			if (item.value != lastValue){
//
//				// if this is the first change - we subtract time from present
//				let interval:TimeInterval
//					= (lastTime == 0) ? now - item.time  : lastTime - item.time
//
//				lastTime = item.time
//				lastValue = item.value
//
//				timeline.append( RESTTimeSpanItem(time: item.time,
//															 durration: interval, value: item.value))
//			}
//		}
//
//		return timeline
//	}
//
//
//}

//enum PumpHouseEvents: Int {
//	case unknown 	= 0
//	case startup		= 1
//	case shutdown	= 2
//	
//	case bypassMode		= 3
//	case inverterMode 	= 4
//	case fastCharge 	 	= 5
//	case floatCharge 	= 6
//	case inverterNotResponding = 7
//	case inverterFail = 8
//	
//	var description : String {
//		switch self {
//		// Use Internationalization, as appropriate.
//		case .unknown: return "Unknown"
//		case .startup: return "Start"
//		case .shutdown: return "Stop"
//		case .bypassMode: return "Bypass Mode"
//		case .inverterMode: return "Inverter Mode"
//		case .fastCharge: return "Fast Charge"
//		case .floatCharge: return "Float Charge"
//		case .inverterNotResponding: return "Not Responding"
//		case .inverterFail: return "Fail"
//		}
//	}
//}
//
//struct RESTEventsTimeSpanItem: Codable {
//	var time:  		Double
//	var durration:  TimeInterval
//	var event: 		Int
//}
//
//struct RESTEventsItem: Codable {
//	var time:  Double
//	var event: Int
//	
//	enum CodingKeys: String, CodingKey {
//		case time = "time"
//		case event = "event"
//	}
//}
//
//struct RESTEvents: Codable {
//	var events:  Array< RESTEventsItem>
//	enum CodingKeys: String, CodingKey {
//		case events = "events"
//	}
//	
//	func timeLine() -> Array<RESTEventsTimeSpanItem> {
//		
//		var timeline:Array<RESTEventsTimeSpanItem> = []
//		
//		let items = self.events.reversed()
//		let now = Date().timeIntervalSince1970
//		var lastTime:Double = 0;
//		
//		for item in items {
//			
//			// if this is the first change - we subtract time from present
//			let interval:TimeInterval
//				= (lastTime == 0) ? now - item.time  : lastTime - item.time
//			
//			lastTime = item.time
//			
//			timeline.append( RESTEventsTimeSpanItem(time: item.time,
//																 durration: interval, event: item.event))
//		}
//		
//		return timeline
//	}
//	
//	
//	func groupedTimeline() -> [[RESTEventsTimeSpanItem]]  {
//		
//		var values:[[RESTEventsTimeSpanItem]] = []
//	
//		var lastOffsset:Int = 1;
//		var i:Int  = -1
//
//		let today = Date()
//		
//		let timeline = self.timeLine()
//		for item in timeline {
//			let date = Date.init(timeIntervalSince1970: item.time)
//			let days = Calendar.current.numberOfDaysBetween(today, and: date)
//		
//			if(days != lastOffsset) {
//				lastOffsset = days
//				values.append([])
//				i+=1
//			}
//			
//			values[i].append(item)
//		}
//		
//		return values
//	}
//	
//}
//

struct RESTEventAction: Codable {
	var cmd			 	:String?
	var action_group 	:String?
	var insteon_group 	:String?
	var deviceID			:String?
	var keypadID			:String?
	var groupID 	:String?
	var level 			:String?
	var value 			:String?

	enum CodingKeys: String, CodingKey {
		case cmd
		case deviceID
		case keypadID
		case groupID
		case level
		case value
		case action_group = "action.group"
		case insteon_group = "insteon.groups"
	}
	
	enum nounClass_t: Int {
		case unknown = 0
		case deviceID
		case groupID
		case keypadID
		case actionGroup
		case insteonGroup
	}
	
	func nounClass() -> nounClass_t{
		
		var result:nounClass_t = .unknown
		
		if deviceID != nil {
			result = .deviceID
		}
		else if groupID != nil {
			result  = .groupID
		}
		else if keypadID != nil {
			result = .keypadID
		}
		else if action_group != nil {
			result  = .actionGroup
			
		} else if insteon_group != nil {
			result  = .insteonGroup
		}
		
		return result
	}
	
	
	
	func noun() -> String{
		var result:String?
		
		switch(self.nounClass()){
		case .deviceID:
			result = deviceID
		case .groupID:
			result = groupID
		case .keypadID:
			result = keypadID
		case .actionGroup:
			result = action_group
		case .insteonGroup:
			result = insteon_group
		case .unknown:
			result = "Unknown"
		}
		return result ?? ""
	}
	
	func nounDescription() -> String{
		var result:String?
		
		switch(self.nounClass()){
		case .deviceID:
			result = "DeviceID"
		case .groupID:
			result = "GroupID"
		case .keypadID:
			result = "KeyPad:"
		case .actionGroup:
			result = "Action Group"
		case .insteonGroup:
			result = "Insteon Group"
		case .unknown:
			result = "Unknown"
			
		}
		return result ?? ""
	}
	
	func image() -> UIImage {
		var image:UIImage? =  nil
		
		switch(self.nounClass()){
		case .deviceID:
			image = UIImage(systemName: "lightbulb")
			
		case .groupID:
			image = UIImage(systemName: "g.circle")
			
		case .keypadID:
			image = UIImage(named: "keypad")
			
		case .actionGroup:
			image = UIImage(systemName: "a.circle")
			
		case .insteonGroup:
			image = UIImage(systemName: "i.circle")
			
		default:
			image =   UIImage(systemName: "questionmark")
		}
		
		return image ?? UIImage()
	}
	
	func verb() -> String{
		var result:String?
		
		if let cmd = cmd {
			
//			if cmd == "set"  && level != nil {
//				result = level?.onLevel().onLevelString()
//			}
//			else if cmd == "backlight"  && level != nil {
//				
//				let levStr =   level!.backLightLevel().backlightLevelString()
//				result =  "backlight \(levStr)"
//			}
//			else if cmd == "keypad.mask"  && value != nil {
//				result = "mask = \(value!)"
//			}
//			else {
//				result = cmd
//			}
		}

		return result ?? ""

	}
}

struct RESTEventTrigger: Codable {
	var mins 				:Int?
	var timeBase			:Int?
	var cmd			 	:String?
	var action_group 	:String?
	var insteon_group 	:String?
	var deviceID			:String?
	var event				:String?

	enum CodingKeys: String, CodingKey {
		case mins
		case timeBase
		case cmd
		case deviceID
		case event
		case action_group = "action.group"
		case insteon_group = "insteon.group"
	}
	
	func triggerDate(_ solarTimes: ServerDateInfo ) ->Date {
		
		var solarTimeFormat: DateFormatter {
			let formatter = DateFormatter()
			formatter.dateFormat = "hh:mm:ss a"
			formatter.timeZone = TimeZone(abbreviation: "UTC")
			return formatter
		}
		
		var date: Date = Date.distantPast
		
		if let sunSet = solarTimes.sunSet,
			let civilSunSet = solarTimes.civilSunSet,
			let civilSunRise = solarTimes.civilSunRise,
			let sunRise = solarTimes.sunRise,
			let midNight = solarTimes.midNight,
			let mins = self.mins {
			
			switch  self.timeBase {
			case 1: // TOD_ABSOLUTE
				date = midNight.addingTimeInterval( Double(mins * 60))
				break
				
			case 2: // TOD_SUNRISE
				date = sunRise.addingTimeInterval( Double(mins * 60))
				break
				
			case 3: // TOD_SUNSET
				date = sunSet.addingTimeInterval( Double(mins * 60))
				break
				
			case 4: // TOD_CIVIL_SUNRISE
				date = civilSunRise.addingTimeInterval( Double(mins * 60))
				break
				
			case 5: // TOD_CIVIL_SUNSET
				date = civilSunSet.addingTimeInterval( Double(mins * 60))
				break
				
			default:
				break
			}
		}
		return date
	}
}


struct RESTEvent: Codable {
	var eventID : String?

	var name		: String
	var action	: 	RESTEventAction
	var trigger	: 	RESTEventTrigger
	
	func isTimedEvent() -> Bool {
		return (self.trigger.timeBase != nil) && (self.trigger.mins != nil)
	}
	
	func isDeviceEvent() -> Bool {
		return (self.trigger.deviceID != nil)
			|| (self.trigger.action_group != nil)
			||  (self.trigger.cmd != nil)
	}
	
	func isAppEvent() -> Bool {
		return (self.trigger.event != nil)
	}
 
	enum eventType: Int {
		case unknown = 0
		case device
		case timed
		case event
	}
	
	enum timedEventTimeBase: Int,CaseIterable {
		case invalid = 0
		case midnight
		case sunrise
		case sunset
		case civilSunrise
		case civilSunset
		
		func description() -> String {
			var str = "Invalid"
			
			switch self {
			case .midnight:		str = "Midnight"
			case .sunrise:  		str = "SunRise"
			case .sunset:  		str = "SunSet"
			case .civilSunrise:  str = "Civil SunRise"
			case .civilSunset:  	str = "Civil SunSet"
			default:
				break
			}
			return str
		}
		
		func image() -> UIImage {
			var image:UIImage? =  nil
			
			switch self {
			case .midnight:		image = UIImage(systemName: "clock")
			case .sunrise:  		image = UIImage(systemName: "sunrise")
			case .sunset: 		 	image = UIImage(systemName: "sunset")
			case .civilSunrise:  image = UIImage(systemName: "sunrise.fill")
			case .civilSunset:  	image = UIImage(systemName: "sunset.fill")
			default:					image =   UIImage(systemName: "questionmark")
			}
			return image ?? UIImage()
		}
	}
	
	func eventType() ->  eventType {
		
		if(self.isTimedEvent()) {
			return .timed
		}
		else 	if(self.isDeviceEvent()) {
			return .device
		}
		else 	if(self.isAppEvent()) {
			return .event
		}
		return .unknown
	}
	
	
	func stringForTrigger() ->  String {
		
		var str = "Invalid"
		
		switch self.eventType() {
		case .timed:
			if let timebase = self.trigger.timeBase,
				let timedTrigger = timedEventTimeBase(rawValue: timebase)   {
				str = timedTrigger.description()
			}
			
		case .device:
			str = "Device"
			
		case .event:
			if self.trigger.event == "startup" {
				str = "Startup"
			}
			break
			
		default:
			break
		}
		
		return str
	}
	
	
	
	func imageForTrigger() -> UIImage {
		
		var image = UIImage(systemName: "questionmark")
	
		switch self.eventType() {
		case .timed:
			if let timebase = self.trigger.timeBase,
				let timedTrigger = timedEventTimeBase(rawValue: timebase)   {
				image = timedTrigger.image()
			}
			
		case .device:
	//		str = "Device"
			break
			
		case .event:
			if self.trigger.event == "startup" {
				image = UIImage(systemName: "power")
			}
			break
			
		default:
			break
		}

		return image ??  UIImage()
	}
}


struct RESTEventTime: Decodable {
	var eventID: String
	var minsFromMidnight: Int
	//var wrappedValue: (String, Int)
	
	//	func encode(to encoder: Encoder) throws {
	//		var unkeyedContainer = encoder.unkeyedContainer()
	//
	//	}
	
	init(from decoder: Decoder) throws {
		
		eventID = ""
		minsFromMidnight = 0
		
		var container = try decoder.unkeyedContainer()
		if (container.count == 2) {
			eventID = try container.decodeIfPresent(String.self)!
			minsFromMidnight = try container.decodeIfPresent(Int.self)!
		}
	}
}
 
struct RESTEventList: Decodable {
	var eventIDs: 			Dictionary<String, RESTEvent>
	var events_timed: 	[RESTEventTime]?
	var future: 			[String]?
	
	enum CodingKeys: String, CodingKey {
		case eventIDs = "eventIDs"
		case events_timed = "events.timed"
		case future = "events.future"
	}
	
//	init(from decoder: Decoder) throws {
//
//		let container = try decoder.container(keyedBy: CodingKeys.self)
//		eventIDs = try container.decode([String: RESTEvent].self, forKey: CodingKeys.eventIDs)
//		future =  try? container.decode([String].self, forKey: CodingKeys.eventIDs)
//
//		let arr = try container.decode(Array<Any>.self, forKey:  CodingKeys.events_timed)
//
//		events_timed = []
////		events_timed  = try? container.decode([RESTEventTime].self, forKey: CodingKeys.events_timed)
//
//	}
}


class CCServerManager: ObservableObject {

	enum ServerError: Error {
		case connectFailed
		case invalidState
		case invalidURL
		case unknown
	}
 
	@Published var lastUpdate = Date()
	
	static let shared: CCServerManager = {
			let instance = CCServerManager()
			// Setup code
			return instance
		}()
	
	init () {
		
	}
	
	
	func calculateSignature(forRequest: URLRequest, apiSecret: String ) -> String {
		
		if let method: String =  forRequest.httpMethod,
			let urlPath = forRequest.url?.path ,
			let daytimeHeader = forRequest.value(forHTTPHeaderField: "X-auth-date"),
			let apiKey = forRequest.value(forHTTPHeaderField: "X-auth-key")
		{
			var bodyHash:String = ""
			
			if let body = forRequest.httpBody {
				bodyHash = body.sha256String()
			}
			else {
				bodyHash = Data().sha256String()
			}
			
			let stringToSign =   method +  "|" + urlPath +  "|"
				+  bodyHash +  "|" + daytimeHeader + "|" + apiKey
  
			let signatureString = stringToSign.hmac(key: apiSecret)
			
				return signatureString;
		}
	
		return "";

	}
	
	func RESTCall(urlPath: String,
					  httpMethod: String? = "GET",
					  headers: [String : String]? = nil,
					  queries: [URLQueryItem]? = nil,
					  body: Any? = nil,
					  timeout:TimeInterval = 10,
					  completion: @escaping (URLResponse?,  Any?, Error?) -> Void)  {
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			if let queries = queries {
				urlComps.queryItems = queries
			}
			var request = URLRequest(url: urlComps.url!)
			
			// Specify HTTP Method to use
			request.httpMethod = httpMethod
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
		
			if let body = body {
				let jsonData = try? JSONSerialization.data(withJSONObject: body)
				request.httpBody = jsonData
			}
 
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			headers?.forEach{
				request.setValue($1, forHTTPHeaderField: $0)
			}
				
			// Send HTTP Request
			request.timeoutInterval = timeout
			
 //	 	print(request)
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				if urlError != nil {
					completion(nil, nil, urlError	)
					return
				}
			
			
 // 			print ( String(decoding: (data!), as: UTF8.self))

				if let data = data as Data? {
					
					let decoder = JSONDecoder()
					
					if let restErr = try? decoder.decode(RESTError.self, from: data){
						completion(response, restErr.error, nil)
					}
					else if let obj = try? decoder.decode(RESTValuesList.self, from: data){
						completion(response, obj, nil)
					}
//					else if let obj = try? decoder.decode(RESTHistory.self, from: data){
//						completion(response, obj, nil)
//					}
					else if let obj = try? decoder.decode(RESTStatus.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDateInfo.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTSchemaList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTProperties.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTEventList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTEvent.self, from: data){
						completion(response, obj, nil)
					}
					else if let jsonObj = try? JSONSerialization.jsonObject(with: data, options: .allowFragments) as? Dictionary<String, Any> {
						completion(response, jsonObj, nil)
					}
					else {
						completion(response, nil, nil)
					}
				}
			}
			
			task.resume()
		}
		else {
			completion(nil,nil, ServerError.invalidURL)
		}
	}
}

