//
//  ScheduleTableHeaderView.swift
//  homecontrol app
//
//  Created by Vincent Moscaritolo on 11/22/21.
//

import UIKit


final class ScheduleTableHeaderView: UITableViewHeaderFooterView {
	static let reuseIdentifier: String = String(describing: self)

	static var nib: UINib {
		 return UINib(nibName: String(describing: self), bundle: nil)
	}

	@IBOutlet var title: UILabel? {
		 get { return _title }
		 set { _title = newValue }
	}
	private var _title: UILabel?
}
