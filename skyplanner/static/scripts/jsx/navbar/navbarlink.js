class NavbarLink extends React.Component {
    constructor(props) {
        super(props);
        this.state = {is_active: ''};
    }
    render() {
        return <li className={this.state.is_active + " navbar-link"}>
                <a href='#' onClick={() => this.setActive(! this.state.is_active )}>{this.props.text}</a>
               </li>;
    }

    setActive(is_active) {
        var state = this.state;
        state.is_active = is_active ? 'active' : '';
        this.setState(state);
    }
}
export default NavbarLink;
