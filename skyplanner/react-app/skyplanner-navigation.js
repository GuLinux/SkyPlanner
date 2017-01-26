import React from 'react';
import { Navbar, Nav, NavItem } from 'react-bootstrap';
import { LinkContainer, IndexLinkContainer } from 'react-router-bootstrap';
import AuthManager from './auth-manager';
import URLs from './urls'

class SkyPlannerNavigation extends React.Component {
    constructor(props) {
        super(props);
        this.state = { loggedIn: !!AuthManager.user() };
    }

    componentDidMount() {
        AuthManager.register(this);
    }

    componentWillUnmount() {
        AuthManager.unregister(this);
    }

    loginChanged(user) {
        this.setState({loggedIn: !!user});
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
                    {this.state.loggedIn ? this.navsAuthenticated() : this.navsAnonymous()}
                </Navbar.Collapse>
            </Navbar>
            ;
    }

    navsAuthenticated() {
        return (
            <Nav>
                <IndexLinkContainer to={URLs.root.path}><NavItem eventKey='index'>Home</NavItem></IndexLinkContainer>
                <LinkContainer to={URLs.logout.path}><NavItem eventKey='logout'>Logout</NavItem></LinkContainer>
                <LinkContainer to={URLs.equipment.path}><NavItem eventKey='equipment'>Equipment</NavItem></LinkContainer>
            </Nav>
        );
    }

    navsAnonymous() {
         return (
            <Nav>
                <IndexLinkContainer to={URLs.root.path}><NavItem eventKey='index'>Home</NavItem></IndexLinkContainer>
                <LinkContainer to={URLs.login.path}><NavItem eventKey='login'>Login</NavItem></LinkContainer>
            </Nav>
        );
   }
};

export default SkyPlannerNavigation;
