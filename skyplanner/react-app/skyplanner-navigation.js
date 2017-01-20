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
                    <Nav activeKey='home'>
                        {this.navs()}
                    </Nav>
                </Navbar.Collapse>
            </Navbar>
            ;
    }

    navs() {
        return this.props.navs.map( (nav) => { return <NavItem key={nav.key} eventKey={nav.key} href={'/' + nav.key}>{nav.display}</NavItem> } );;
    }
};

export default SkyPlannerNavigation;
