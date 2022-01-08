//
//  ScheduleDetailView.swift
//  homecontrol app
//
//  Created by Vincent Moscaritolo on 11/24/21.
//

import UIKit
import Toast
//import PureLayout

import AnyFormatKit


// MARK:- TriggerViewController

public protocol TriggerViewControllerDelegate  {
	func triggerViewChanged()
}
 
class TriggerViewController: UIViewController {
	
	var event: RESTEvent? = nil
	var delegate:TriggerViewControllerDelegate? = nil
}

// MARK:- TriggerEventViewController

class TriggerEventViewController: TriggerViewController {

	@IBOutlet var btnEvent	: UIButton!

	class func create(withEvent: RESTEvent) -> TriggerEventViewController? {
		let storyboard = UIStoryboard(name: "ScheduleDetailView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "TriggerEventViewController") as? TriggerEventViewController
		
		if let vc = vc {
			vc.event = withEvent
		}
		
		return vc
	}

	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
	
		if let evt = event {
			// reset the event type if its wrong
			if(!evt.isAppEvent()) {
				self.event?.trigger = RESTEventTrigger()
				self.event?.trigger.event = RESTEvent.appEventTrigger.startup.rawValue
				self.delegate?.triggerViewChanged()

			}
		}
 
		btnEvent.setTitle(event?.stringForTrigger(), for: .normal	)
		var actions: [UIAction] = []
		
		for eventTypes in  RESTEvent.appEventTrigger.allCases {
				if(eventTypes == .invalid){
				continue
			}
	
			let item = UIAction(title: eventTypes.description(),
									  image:  nil, //timebase.image(),
									  identifier: UIAction.Identifier( "\(eventTypes.rawValue)")
			
			) { (action) in
	
					if let newEvtType = Int(action.identifier.rawValue){
//					if (newEvtType != self.event?.trigger.event ){
//
					
//						self.event?.trigger.timeBase = Int(action.identifier.rawValue)
 						self.refreshView()
 						self.delegate?.triggerViewChanged()
//					}
 				}

//				self.event?.trigger.timeBase = Int(action.identifier.rawValue)
//				self.refreshView()
			}

			actions.append(item)
		}

		let menu = UIMenu(title: "Event Trigger",
								 options: .displayInline, children: actions)
		
		btnEvent.menu = menu
		btnEvent.showsMenuAsPrimaryAction = true
		
	
	}
	
	func refreshView() {
		btnEvent.setTitle(event?.stringForTrigger(), for: .normal	)
	}
	
}

// MARK:- TriggerTimeViewController

class TriggerTimeViewController: TriggerViewController, UITextFieldDelegate

{
	@IBOutlet var btnTimeBase	: UIButton!
	@IBOutlet var btnPlusMinus	: UIButton!
	@IBOutlet var txtOffset	: UITextField!
	@IBOutlet var lblActualTime: UILabel!
	var solarTimes : ServerDateInfo? = nil

	var solarTimeFormat: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateFormat = "hh:mm a"
		formatter.timeZone = TimeZone(abbreviation: "UTC")
		return formatter
	}
	class func create(withEvent: RESTEvent) -> TriggerTimeViewController? {
		let storyboard = UIStoryboard(name: "ScheduleDetailView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "TriggerTimeViewController") as? TriggerTimeViewController
		
		if let vc = vc {
			vc.event = withEvent
		}
		
		return vc
	}

	let phoneFormatter = DefaultTextInputFormatter(textPattern: "##:##")
	let phoneInputController = TextFieldInputController()

	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		phoneInputController.formatter = phoneFormatter
	
		txtOffset.addTarget(self, action: #selector(textFieldDidChange), for: .editingChanged)
	}
		
	func refreshBtnPlusMinus() {
		
		let mins =  event?.trigger.mins ?? 0
		if(mins < 0) {
			self.btnPlusMinus.setImage(UIImage(systemName: "minus"), for: .normal)
		}
		else {
			self.btnPlusMinus.setImage(UIImage(systemName: "plus"), for: .normal)
		}
		
	}
	
	
	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
	
		if let evt = event {
			// reset the event type if its wrong
			if(!evt.isTimedEvent()) {
				self.event?.trigger = RESTEventTrigger()
				self.event?.trigger.timeBase = RESTEvent.timedEventTimeBase.midnight.rawValue
				self.event?.trigger.mins = 0
	 			self.delegate?.triggerViewChanged()
			}
		}
		
		txtOffset.delegate = self
		
		ChickenCoop.shared.fetchData(.date) { result in
	 		if case .success(let solarTimes as ServerDateInfo) = result {
				self.solarTimes = solarTimes
			}
			else {
				self.solarTimes = nil
			}
			self.refreshActualTime()
		}
 
		txtOffset.text = minsToString(event?.trigger.mins ?? 0)
	
		let menu1 = UIMenu(title: "Offset ",
								 options: .displayInline, children: [
									
									UIAction(title: "plus", image: UIImage(systemName: "plus"),identifier: nil
												
									) { (action) in
										
										let mins =  self.event?.trigger.mins ?? 0
										if(mins < 0){
											self.event?.trigger.mins  = abs(mins)
											self.refreshBtnPlusMinus()
											self.refreshActualTime()
											self.delegate?.triggerViewChanged()
										}
									},
									UIAction(title: "minus", image: UIImage(systemName: "minus"), identifier: nil
												
									) { (action) in
										
										let mins =  self.event?.trigger.mins ?? 0
										if(mins > 0){
											self.event?.trigger.mins  = -mins
											self.refreshBtnPlusMinus()
											self.refreshActualTime()
											self.delegate?.triggerViewChanged()
										}
									}
								])
									 
		
		btnPlusMinus.menu = menu1
		btnPlusMinus.showsMenuAsPrimaryAction = true
		self.refreshBtnPlusMinus()

		btnTimeBase.setTitle(event?.stringForTrigger(), for: .normal	)
		btnTimeBase.setImage(event?.imageForTrigger(), for: .normal)
		var actions: [UIAction] = []
		
		for timebase in [ 	RESTEvent.timedEventTimeBase.civilSunrise,
									RESTEvent.timedEventTimeBase.sunrise,
									RESTEvent.timedEventTimeBase.sunset,
									RESTEvent.timedEventTimeBase.civilSunset] {
//	in RESTEvent.timedEventTimeBase.allCases {
//			if(timebase == .invalid){
//				continue
//			}
//
			let item = UIAction(title: timebase.description(),
									  image: timebase.image(),
									  identifier: UIAction.Identifier( "\(timebase.rawValue)")
			
			) { (action) in
		
				self.txtOffset.resignFirstResponder()
	 
					if let newTimeBase = Int(action.identifier.rawValue){
					if (newTimeBase != self.event?.trigger.timeBase ){
						
						self.event?.trigger.timeBase = Int(action.identifier.rawValue)
						self.refreshView()
						self.delegate?.triggerViewChanged()
					}
				}

				self.event?.trigger.timeBase = Int(action.identifier.rawValue)
				self.refreshView()
			}

			actions.append(item)
		}

		let menu = UIMenu(title: "Timebase for Event",
								 options: .displayInline, children: actions)
		
		btnTimeBase.menu = menu
		btnTimeBase.showsMenuAsPrimaryAction = true
		
	
	}
	
	func refreshView() {
		btnTimeBase.setTitle(event?.stringForTrigger(), for: .normal)
		btnTimeBase.setImage(event?.imageForTrigger(), for: .normal)
		refreshActualTime()
	}
	
	func refreshActualTime() {
		
		if let st = solarTimes {
			if let date = self.event?.trigger.triggerDate(st) {
				lblActualTime.text = solarTimeFormat.string(from: date)
			}
		}
		else {
			lblActualTime.text = "Fail"
		}
	}
	

	func minsToString(_ mins: Int) ->String {
		let mins = abs(mins)
		let hours = Int(mins) / 60
		let minutes =  mins - (hours * 60)
 		return  String(format:"%02i:%02i", hours, minutes)
	}
	

	func offsetMins() -> Int {
		// write code to take offset and update the event mins..
		
		return 0
	}
	
	@objc final private func textFieldDidChange(textField: UITextField) {
		 
		if let  items = textField.text?.split(separator: ":"),
			let mins = self.event?.trigger.mins,
			items.count == 2 {
			
			var newMins:Int = 0
			newMins = (Int(items[0]) ?? 0) * 60
			newMins +=  Int(items[1]) ?? 0
			if mins < 0 {
				newMins = newMins * -1
			}
			
			if let event = self.event,
				event.trigger.mins != newMins{
				self.event?.trigger.mins = newMins
				self.delegate?.triggerViewChanged()
				self.refreshActualTime()
				
			}
			
		}
	}
	
	
	func textFieldDidEndEditing(_ textField: UITextField){
		
		textFieldDidChange(textField:textField)
//		if let  items = textField.text?.split(separator: ":"),
//			let mins = self.event?.trigger.mins,
//			items.count == 2 {
//
//			var newMins:Int = 0
//			newMins = (Int(items[0]) ?? 0) * 60
//			newMins +=  Int(items[1]) ?? 0
//			if mins < 0 {
//				newMins = newMins * -1
//			}
//
//			if let event = self.event,
//				event.trigger.mins != newMins{
//				self.event?.trigger.mins = newMins
//				self.delegate?.triggerViewChanged()
//				self.refreshActualTime()
//
//			}
//
//		}
		
	}

	
	func textFieldShouldReturn(_ textField: UITextField) -> Bool {
		if(textField == txtOffset)
		{
			textField.resignFirstResponder()
		}
		return true
	}
	
	
	func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool  {

		guard  textField == txtOffset else {
			return true
		}
		
		guard string.rangeOfCharacter(from: CharacterSet(charactersIn: "1234567890:").inverted) == nil
			else {
				return false;
			}
	 
		return phoneInputController.textField(textField, shouldChangeCharactersIn: range, replacementString: string)
	}

 }

// MARK:- ScheduleDetailViewController

class ScheduleDetailViewController :UIViewController,
												EditableUILabelDelegate,
												TriggerViewControllerDelegate{
	
	
	@IBOutlet var lblTitle	: EditableUILabel!

	@IBOutlet var segEvent	: UISegmentedControl!
	
	@IBOutlet var vwEvent	: UIView!
	@IBOutlet var vwAction	: UIView!
	
	@IBOutlet var btnCmd	: UIButton!
	@IBOutlet var btnDevice	: UIButton!
	 
	@IBOutlet var btnSave	: UIButton!

	var delegate:SchedulesViewController? = nil
	
	var currentEventVC : TriggerViewController? = nil
	var event: RESTEvent? = nil
	var eventType:RESTEvent.eventType = .unknown
	var didChangeEvent:Bool = false
	
	var eventID :String = ""
	
	class func create(withEventID: String) -> ScheduleDetailViewController? {
		let storyboard = UIStoryboard(name: "ScheduleDetailView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "ScheduleDetailViewController") as? ScheduleDetailViewController
		
		if let vc = vc {
			vc.eventID = withEventID
		}
		
		return vc
	}
	
	class func newEvent() -> ScheduleDetailViewController? {
		let storyboard = UIStoryboard(name: "ScheduleDetailView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "ScheduleDetailViewController") as? ScheduleDetailViewController
		
		if let vc = vc {
			vc.eventID = ""
		}
		
		return vc
	}


	
	// MARK:-  view
	
	override func viewDidLoad() {
		super.viewDidLoad()
		lblTitle.delegate = self
		
		// Add function to handle Value Changed events
		segEvent.addTarget(self, action: #selector(self.segmentedValueChanged(_:)), for: .valueChanged)
	}
	
	
	
	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
	
		if(eventID.isEmpty){
			createNewEvent();
		}
 		else {
			reloadEvent()
		}
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		
		if let firstVC = SchedulesViewController.shared {
			DispatchQueue.main.async {
				
				firstVC.refreshSchedules()
			}
		}
	}
	
	
	func setupSubviewsForEvent(){
		
		var newVC:TriggerViewController? = nil
		
		if let event = event {
			self.lblTitle.text = event.name

			switch(eventType){
			
			case .timed:
				segEvent.selectedSegmentIndex = 0
				newVC = TriggerTimeViewController.create(withEvent: event)
	 
			case .event:
				segEvent.selectedSegmentIndex = 1
				newVC = TriggerEventViewController.create(withEvent: event)
				
			default:
				break
			}
			
			if(newVC == currentEventVC){
				return
			}
			
			if let cvc = currentEventVC {
				cvc.willMove(toParent: nil)
				cvc.view.removeFromSuperview()
				cvc.removeFromParent()
				currentEventVC = nil
			}
			
			if let newVC = newVC {
				currentEventVC = newVC
				newVC.delegate = self

				newVC.view.frame = self.vwEvent.bounds
				newVC.willMove(toParent: self)
				self.vwEvent.addSubview(newVC.view)
				self.addChild(newVC)
				newVC.didMove(toParent: self)
			}
		}
	}
		
	func refreshCmd() {
		
		var actions1: [UIAction] = []
		
		if let event = self.event{
			let cmds = 	event.allCasesForDeviceID(devID:event.deviceIDValue())
			for cmd in cmds{
				if(cmd == .invalid){
					continue
				}
				let item = UIAction(title: cmd.description(),
										  image: nil,
										  identifier: UIAction.Identifier( "\(cmd.rawValue)")
				) { (action) in
					
					let newCmd =  action.identifier.rawValue
					if (newCmd != self.event?.action.cmd ){
						//
						self.event?.action.cmd  = newCmd
						self.refreshAction()
						self.triggerViewChanged()
					}
				}
				actions1.append(item)
			}
		}
 
	 
		let menu1 = UIMenu(title: "Device Command",
								options: .displayInline, children: actions1)
		
		btnCmd.menu = menu1
		btnCmd.showsMenuAsPrimaryAction = true
		btnCmd.setTitle(event?.stringForActionCmd(), for: .normal	)
		btnCmd.isEnabled	=  event?.deviceIDValue() != .invalid
		
	}
	
	func refreshDevice() {
		
		var actions2: [UIAction] = []
		for deviceID in RESTEvent.actionDeviceID.allCases {
			if(deviceID == .invalid){
				continue
			}

			let item = UIAction(title: deviceID.description(),
									  image: nil,
									  identifier: UIAction.Identifier( "\(deviceID.rawValue)")

			) { (action) in
				
				if var event = self.event{
					
					let newDeviceID =  action.identifier.rawValue
					if (newDeviceID != event.action.deviceID ){
						//
						event.action.deviceID  = newDeviceID
						self.btnDevice.setTitle(event.stringForActionDeviceID(), for: .normal	)

						self.event = event.normalizedEvent()
						self.refreshCmd()
						self.triggerViewChanged()
					}

				}
			}

			actions2.append(item)
		}

		let menu2 = UIMenu(title: "Device ID",
								options: .displayInline, children: actions2)

		btnDevice.menu = menu2
		btnDevice.showsMenuAsPrimaryAction = true
		btnDevice.setTitle(event?.stringForActionDeviceID(), for: .normal	)
 
	}

	func refreshAction(){

		refreshCmd()
		refreshDevice()
	}
	
	
	func createNewEvent(){
		event = RESTEvent()
		event?.name = "Untitled Event"
		event?.trigger.timeBase = 1
		
		self.eventType = event!.eventType()

		self.refreshAction()
		self.setupSubviewsForEvent()
		
		self.btnSave.isEnabled = false
	}
	
	
	func reloadEvent() {
		ChickenCoop.shared.fetchData(.event, ID: self.eventID) { result in
			if case .success(let evt as RESTEvent) = result {
				
				DispatchQueue.main.async{
					
					self.btnSave.isEnabled = false
					
					self.event = evt
					self.eventType = evt.eventType()
					
					self.refreshAction()
					self.setupSubviewsForEvent()
				}
			}
		}
	}
	
	// MARK:- segment control
	
	@objc func segmentedValueChanged(_ sender:UISegmentedControl!) {
		switch( sender.selectedSegmentIndex) {
		case 0:
			eventType = .timed
			
		case 1:
			eventType = .event
			
		default: break
			
		}
		
		didChangeEvent = true
		btnSave.isEnabled = true
		setupSubviewsForEvent()
	}

	// MARK: -


	
	@IBAction func btnSaveClicked(_ sender: Any) {
		if  eventID == "" || event?.eventID == nil {
	 
			// new event!
			CCServerManager.shared.createEvent(event!)  { error, eventID  in
				if(error == nil){
					self.delegate?.eventChanged()
					self.dismiss(animated: true)
				}
				else {
					Toast.text(error?.localizedDescription ?? "Error",
								  config: ToastConfiguration(
									autoHide: true,
									displayTime: 1.0
									//												attachTo: self.vwError
								  )).show()
				}
			}
		}
		else {
			
			// save changes
			CCServerManager.shared.updateEvent(event!)  { error  in
				if(error == nil){
					self.delegate?.eventChanged()
					self.dismiss(animated: true)
				}
				else {
					Toast.text(error?.localizedDescription ?? "Error",
								  config: ToastConfiguration(
									autoHide: true,
									displayTime: 1.0
									//												attachTo: self.vwError
								  )).show()

				}
				
			}
		}
		
	}
	
	func triggerViewChanged(){
		if let trigger = currentEventVC?.event?.trigger {
			event?.trigger = trigger
		}
		
		btnCmd.isEnabled	=  event?.deviceIDValue() != .invalid
		btnSave.isEnabled = event!.isValid()
	}
	
	func renameEvent(newName:String){

		event?.name = newName
		btnSave.isEnabled = event!.isValid()
	}
	
	func editMenuTapped(sender: UILabel) {
		
		let alert = UIAlertController(title:  NSLocalizedString("Rename Event", comment: ""),
												message: nil,
												cancelButtonTitle: NSLocalizedString("Cancel", comment: ""),
												okButtonTitle:  NSLocalizedString("Rename", comment: ""),
												validate: .nonEmpty,
												textFieldConfiguration: { textField in
			textField.placeholder =  "Event Name"
			textField.text = self.lblTitle.text
		}) { result in
			
			switch result {
			case let .ok(String:newName):
				self.renameEvent( newName: newName);
				break
				
			case .cancel:
				break
			}
		}
		
		// Present the alert to the user
		self.present(alert, animated: true, completion: nil)
		}
	


}
