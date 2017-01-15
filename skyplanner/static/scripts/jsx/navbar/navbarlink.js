class NavbarLink extends React.Component {
    constructor(props) {
        super(props);
        this.state = {is_active: ''};
    }
    render() {
        console.log(this.props);
        return <li className={this.state.is_active + " navbar-link"}>
                <a href='#' onClick={this.onClick}>{this.props.text}</a>
               </li>;
    }

    setActive(is_active) {
        var state = this.state;
        state.is_active = is_active ? 'active' : '';
        this.setState(state);
    }

    onClick() {
        if(this.state.onClickHandler !== undefined)
            this.state.onClickHandler(this);
        this.setActive(true);
    }

    setOnClickHandler(handler) {
        this.state.onClickHandler = handler;
    }
}
export default NavbarLink;
