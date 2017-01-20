import React from 'react';
import { Navbar, Nav, NavItem } from 'react-bootstrap';

class SkyPlannerNavigation extends React.Component {
    constructor(props) {
        super(props);
    }
    render() {
        return <Navbar inverse collapseOnSelect fluid>
                <Navbar.Header>
                    <Navbar.Brand>
                        <a href="#">SkyPlanner</a>
                    </Navbar.Brand>
                    <Navbar.Toggle />
                </Navbar.Header>
                <Navbar.Collapse>
                    <Nav>
                        <NavItem eventKey={1} href="#">Link</NavItem>
                    </Nav>
                </Navbar.Collapse>
            </Navbar>
            ;
    }
};

export default SkyPlannerNavigation;
