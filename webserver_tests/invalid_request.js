import axios from 'axios'
const base_url = "http://localhost:8888"

axios.patch(`${base_url}/`).then(response => {
    // Since patch is not supported by the server it returns bad request
    console.log("Should not be a successful response; This should never print!!!!")
}).catch(e => {
    console.log(e.response.status, e.response.data)
})

