//
//  InfoViewController.swift
//  homecontrol app
//
//  Created by Vincent Moscaritolo on 11/18/21.
//

import Foundation
import UIKit

class InfoViewController: UIViewController  {
	
	@IBOutlet var lblStatus	: UILabel!
	@IBOutlet var lblCPUTemp	: UILabel!
	@IBOutlet var btnStartStop : BHButton!

	@IBOutlet var lblVersion		: UILabel!
	@IBOutlet var lblBuildDate	: UILabel!

	@IBOutlet var lblLat		: UILabel!
	@IBOutlet var lblLong		: UILabel!
	@IBOutlet var lblSunSet	: UILabel!
	@IBOutlet var lblSunSet1	: UILabel!
	
	@IBOutlet var lblSunRise	: UILabel!
	@IBOutlet var lblSunRise1	: UILabel!

	@IBOutlet var lblNodeName	: UILabel!
	@IBOutlet var lblOSName	: UILabel!
	@IBOutlet var lblOSVersion	: UILabel!
	@IBOutlet var lblHW			: UILabel!

	@IBOutlet var lblLocalTime	: UILabel!
	
	@IBOutlet var lblUptime	: UILabel!
	
	var coopstate: device_state_t = .DEVICE_STATE_UNKNOWN

	var timer = Timer()
	
	var solarTimeFormat: DateFormatter {
		 let formatter = DateFormatter()
		formatter.dateFormat = "hh:mm:ss a"
		formatter.timeZone = TimeZone(abbreviation: "UTC")
		return formatter
	}

	var upTimeFormatter: DateComponentsFormatter {
		let formatter = DateComponentsFormatter()
		formatter.allowedUnits = [.day, .hour, .minute, .second]
		formatter.unitsStyle = .short
		return formatter
	}
	 
	var gmtDateFormatter: DateFormatter {
		let formatter = DateFormatter()
	  formatter.dateFormat =  "EEE',' dd MMM yyyy HH':'mm':'ss z"
	  return formatter
	}
	
	var localDateFormatter: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateStyle = .short
		formatter.timeStyle = .medium
	  return formatter
	}

	func localTimeFromGMT(_ gmtString: String) ->String {
	
		var result:String = ""
		if let serverDate = gmtDateFormatter.date(from: gmtString){
			result = localDateFormatter.string(from: serverDate)
		}
		return result;
	}

	class func create() -> InfoViewController? {
		let storyboard = UIStoryboard(name: "InfoView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "InfoViewController") as? InfoViewController
		
		return vc
	}

	override func viewDidLoad() {
		super.viewDidLoad()
	}
	
	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
		
		if(AppData.serverInfo.validated){
			startPolling();
		}
		else {
			stopPollng();
		}
		
		
		refreshView()
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		stopPollng();
		
	}


	private func refreshView() {
		guard AppData.serverInfo.validated  else {
			return
		}
		
		ChickenCoop.shared.fetchData(.status) { result in
//			dp.leave()
			if case .success(let status as RESTStatus) = result {
				
				self.coopstate =	device_state_t(rawValue: status.state) ?? .DEVICE_STATE_UNKNOWN
	
				switch self.coopstate {
					
				case .DEVICE_STATE_CONNECTED:
					self.btnStartStop.isEnabled = true
					self.btnStartStop.setTitle("Stop", for: .normal)
					
					
				default:
					self.btnStartStop.isEnabled = true
					self.btnStartStop.setTitle("Start", for: .normal)
					
					break
				}
				
				self.lblVersion.text = status.version
				self.lblBuildDate.text = status.buildtime
				
				self.lblNodeName.text = status.os_nodename
				self.lblOSName.text = status.os_sysname
				self.lblOSVersion.text = status.os_release
				self.lblHW.text = status.os_machine
							
				self.lblLocalTime.text = self.localTimeFromGMT(status.date)
				
				let upTimeStr = self.upTimeFormatter.string(from: TimeInterval(status.uptime))!
				self.lblUptime.text = upTimeStr
				self.lblStatus.text = status.stateString
				
				if let cpuTemp = status.cpuTemp {
					self.lblCPUTemp.text =  String(format: "%.0f°C", cpuTemp )
				}
		
			}
			else {
				self.lblVersion.text 	= ""
				self.lblBuildDate.text = ""
				self.lblNodeName.text 	= ""
				self.lblOSName.text	 = ""
				self.lblOSVersion.text = ""
				self.lblHW.text 		= ""
				self.lblLocalTime.text = ""
				self.lblStatus.text 	= ""
				self.lblUptime.text 	= ""
				
				self.btnStartStop.isEnabled = false
				self.btnStartStop.setTitle("Error", for: .normal)
			
			}
		}
		
	//	dp.enter()
		ChickenCoop.shared.fetchData(.date) { result in
//			dp.leave()
			
			self.lblLat.text 	= "Not Set"
			self.lblLong.text = ""
			self.lblSunSet.text 	= ""
			self.lblSunSet1.text 	= ""
			self.lblSunRise.text 	= ""
			self.lblSunRise1.text 	= ""

			if case .success(let data as ServerDateInfo) = result {
				
				if let latitude = data.latitude ,
					let longitude = data.longitude,
					let sunSet = data.sunSet,
					let civilSunSet = data.civilSunSet,
					let civilSunRise = data.civilSunRise,
					let sunRise = data.sunRise
				{
					self.lblLat.text = String(latitude)
					self.lblLong.text = String(longitude)
					self.lblSunSet.text = self.solarTimeFormat.string(from: sunSet)
					self.lblSunSet1.text = self.solarTimeFormat.string(from: civilSunSet)
					self.lblSunRise.text = self.solarTimeFormat.string(from: sunRise)
					self.lblSunRise1.text = self.solarTimeFormat.string(from: civilSunRise)
				}
			 
			
	 		}
		
		}
		
	}
	
	public func startPolling() {
		timer =  Timer.scheduledTimer(withTimeInterval: 1.0,
												repeats: true,
												block: { timer in
													self.refreshView()
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}

	
	@IBAction func StartStopClicked(_ sender: UIButton) -> Void {

		switch self.coopstate {

		case .DEVICE_STATE_CONNECTED:
			stopCoop()

		default:
			startCoop()

			break
		}
	}


	func startCoop() {

		CCServerManager.shared.RESTCall(urlPath: "state",
												 httpMethod: "PUT",
												 body: ["state":"start"] )
		{ (response, json, error)  in
				self.refreshView()
		}

	}

	func stopCoop() {

		CCServerManager.shared.RESTCall(urlPath: "state",
												 httpMethod: "PUT",
												 body: ["state":"stop"] )
		{ (response, json, error)  in
				self.refreshView()
		}
	}

}
	 
