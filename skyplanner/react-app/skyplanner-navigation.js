import React from 'react';
import { Navbar, Nav, NavItem } from 'react-bootstrap';
import { LinkContainer, IndexLinkContainer } from 'react-router-bootstrap';


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
                        {this.navs()}
                    </Nav>
                </Navbar.Collapse>
            </Navbar>
            ;
    }

    navs() {
        return this.props.navs.map( (nav) => {
            if(nav.key == '/')
                return (
                    <IndexLinkContainer key={nav.key} to={nav.key}>
                        <NavItem eventKey={nav.key}>{nav.display}</NavItem>
                    </IndexLinkContainer>

                );
            return (
                <LinkContainer key={nav.key} to={nav.key}>
                    <NavItem eventKey={nav.key}>{nav.display}</NavItem>
                </LinkContainer>
            );
        } );
    }
};

export default SkyPlannerNavigation;
