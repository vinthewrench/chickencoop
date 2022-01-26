//
//  CoopValueHistoryController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/12/21.
//

import Foundation
import UIKit
import Toast

final class CoopValueHistoryCell: UITableViewCell {
	
	@IBOutlet var lblTitle	: UILabel!
	@IBOutlet var lblSubTitle	: UILabel!
	@IBOutlet var lblValue	: UILabel!
 
	override func awakeFromNib() {
		super.awakeFromNib()
	}
}


class CoopValueHistoryController: 	UIViewController,										  										UITableViewDelegate,
												 UITableViewDataSource   {
	
	// cell reuse id (cells that scroll out of view can be reused)
	let cellReuseIdentifier = "CoopValueHistoryCell"
	
	
	@IBOutlet var lblTitle	: UILabel!
	@IBOutlet var lblSubTitle	: UILabel!
	@IBOutlet var lblCount	: UILabel!
	@IBOutlet var tableView: UITableView!
	
	@IBOutlet var btnClear: UIButton!
	
	var valueKey  = String()
	var schema :RESTSchemaDetails =  RESTSchemaDetails()
	var history: Array< RESTHistoryItem> = []
	
	var valueFormatter: CoopValueFormatter  {
		let formatter = CoopValueFormatter()
		formatter.units = RESTschemaUnits(rawValue:schema.units) ?? .UNKNOWN
		return formatter
	}
	
	var timeFormatter: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateStyle = .short
		formatter.timeStyle = .short
		formatter.doesRelativeDateFormatting = true
		
		return formatter
	}
	
	var timer = Timer()
	
	// MARK: - view lifetime
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		// (optional) include this line if you want to remove the extra empty cell divider lines
		self.tableView.tableFooterView = UIView()
		tableView.delegate = self
		tableView.dataSource = self
	}
	
	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
		
		lblTitle.text = "History: " + self.schema.name
		lblSubTitle.text = self.valueKey
		lblCount.isHidden = true
		
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
		//		refreshButtons(.unknown);
	}
	
	// MARK: - polling / refresh
	
	public func startPolling() {
		
		timer =  Timer.scheduledTimer(withTimeInterval: 0.8,
												repeats: true,
												block: { timer in
													DispatchQueue.main.async  {
														self.refreshView()
													}
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}
	
	
	// MARK: - view refresh
	
	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			
			ChickenCoop.shared.fetchHistory(valueKey) { result in
				
				if case .success(let hist as RESTHistory ) = result {
					var items = hist.values
					
					items.sort{
						$0.time > $1.time
					}
					
					self.lblCount.isHidden = false
					self.lblCount.text = String(items.count)
					self.history = items
					
				}
				else {
					self.tableView.reloadData()
					self.history = []
					self.lblCount.isHidden = true
					
				}
				self.tableView.reloadData()
				
			}
		}
	}
	
	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.history.count
		
	}
	
	func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
		
		if let cell = tableView.dequeueReusableCell(withIdentifier:cellReuseIdentifier) as? CoopValueHistoryCell{
			
			cell.accessoryType = .none
			cell.selectionStyle = .none
			
			let val = self.history[indexPath.row]
			let timeStr = self.timeFormatter.string(from: Date(timeIntervalSince1970: val.time))
			cell.lblSubTitle.text = String(format: "%@", timeStr )			
			cell.lblTitle.text = valueFormatter.string(for: val.value)
			
			return cell
		}
		
		return UITableViewCell()
	}
	
	
	// MARK: - actions
	
	@IBAction func btnClearPressed(_ sender: UIButton) -> Void {
		
		verifyDeleteHistory()
		//		stopPollng()
		//
		//		DispatchQueue.main.async  {
		//			ChickenCoop.shared.setDoor(true) {_ in
		//				self.startPolling()
		//			}
		//		}
	}
	
	func verifyDeleteHistory() {
		
		stopPollng()
		
		// create the alert
		let alert = UIAlertController(title: "Erase History",
												message: "Are you sure you want to erase the history for \(self.schema.name).",
												preferredStyle: UIAlertController.Style.alert)
		
		let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: { _ in
			self.startPolling()
		})
		
		let deleteAction = UIAlertAction(title: "Delete", style: .destructive, handler: { _ in
			
			self.view.displayActivityIndicator(shouldDisplay: false)
			
			ChickenCoop.shared.deleteHistoryForValue(self.valueKey)
			{ (error)  in
				self.view.displayActivityIndicator(shouldDisplay: false)
				self.startPolling()
				
				if(error == nil){
					self.refreshView()
				}
				else {
					
					Toast.text(error?.localizedDescription ?? "Error",
								  config: ToastConfiguration(
									autoHide: true,
									displayTime: 1.0,
									attachTo: self.view
								  )).show()
				}
			}
			
		})
		
		alert.addAction(cancelAction)
		alert.addAction(deleteAction)
		
		// show the alert
		self.present(alert, animated: true, completion: nil)
		
	}
	
}
