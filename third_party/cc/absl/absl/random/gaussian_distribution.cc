// BEGIN GENERATED CODE; DO NOT EDIT
// clang-format off

#include "third_party/cc/absl/absl/random/gaussian_distribution.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal {

const gaussian_distribution_base::Tables
    gaussian_distribution_base::zg_ = {
    {3.7130862467425505, 3.442619855899000214, 3.223084984581141565,
     3.083228858216868318, 2.978696252647779819, 2.894344007021528942,
     2.82312535054891045, 2.761169372387176857, 2.706113573121819549,
     2.656406411261359679, 2.610972248431847387, 2.56903362592493778,
     2.530009672388827457, 2.493454522095372106, 2.459018177411830486,
     2.426420645533749809, 2.395434278011062457, 2.365871370117638595,
     2.337575241339236776, 2.310413683698762988, 2.284274059677471769,
     2.25905957386919809, 2.234686395590979036, 2.21108140887870297,
     2.188180432076048731, 2.165926793748921497, 2.144270182360394905,
     2.123165708673976138, 2.102573135189237608, 2.082456237992015957,
     2.062782274508307978, 2.043521536655067194, 2.02464697337738464,
     2.006133869963471206, 1.987959574127619033, 1.970103260854325633,
     1.952545729553555764, 1.935269228296621957, 1.918257300864508963,
     1.901494653105150423, 1.884967035707758143, 1.868661140994487768,
     1.852564511728090002, 1.836665460258444904, 1.820952996596124418,
     1.805416764219227366, 1.790046982599857506, 1.77483439558606837,
     1.759770224899592339, 1.744846128113799244, 1.730054160563729182,
     1.71538674071366648, 1.700836618569915748, 1.686396846779167014,
     1.6720607540975998, 1.657821920954023254, 1.643674156862867441,
     1.629611479470633562, 1.615628095043159629, 1.601718380221376581,
     1.587876864890574558, 1.574098216022999264, 1.560377222366167382,
     1.546708779859908844, 1.533087877674041755, 1.519509584765938559,
     1.505969036863201937, 1.492461423781352714, 1.478981976989922842,
     1.465525957342709296, 1.452088642889222792, 1.438665316684561546,
     1.425251254514058319, 1.411841712447055919, 1.398431914131003539,
     1.385017037732650058, 1.371592202427340812, 1.358152454330141534,
     1.34469275175354519, 1.331207949665625279, 1.317692783209412299,
     1.304141850128615054, 1.290549591926194894, 1.27691027356015363,
     1.263217961454619287, 1.249466499573066436, 1.23564948326336066,
     1.221760230539994385, 1.207791750415947662, 1.193736707833126465,
     1.17958738466398616, 1.165335636164750222, 1.150972842148865416,
     1.136489852013158774, 1.121876922582540237, 1.107123647534034028,
     1.092218876907275371, 1.077150624892893482, 1.061905963694822042,
     1.046470900764042922, 1.030830236068192907, 1.014967395251327842,
     0.9988642334929808131, 0.9825008035154263464, 0.9658550794011470098,
     0.9489026255113034436, 0.9316161966151479401, 0.9139652510230292792,
     0.8959153525809346874, 0.8774274291129204872, 0.8584568431938099931,
     0.8389522142975741614, 0.8188539067003538507, 0.7980920606440534693,
     0.7765839878947563557, 0.7542306644540520688, 0.7309119106424850631,
     0.7064796113354325779, 0.6807479186691505202, 0.6534786387399710295,
     0.6243585973360461505, 0.5929629424714434327, 0.5586921784081798625,
     0.5206560387620546848, 0.4774378372966830431, 0.4265479863554152429,
     0.3628714310970211909, 0.2723208648139477384, 0},
    {0.001014352564120377413, 0.002669629083880922793, 0.005548995220771345792,
     0.008624484412859888607, 0.01183947865788486861, 0.01516729801054656976,
     0.01859210273701129151, 0.02210330461592709475, 0.02569329193593428151,
     0.02935631744000685023, 0.03308788614622575758, 0.03688438878665621645,
     0.04074286807444417458, 0.04466086220049143157, 0.04863629585986780496,
     0.05266740190305100461, 0.05675266348104984759, 0.06089077034804041277,
     0.06508058521306804567, 0.06932111739357792179, 0.07361150188411341722,
     0.07795098251397346301, 0.08233889824223575293, 0.08677467189478028919,
     0.09125780082683036809, 0.095787849121731522, 0.1003644410286559929,
     0.1049872554094214289, 0.1096560210148404546, 0.1143705124488661323,
     0.1191305467076509556, 0.1239359802028679736, 0.1287867061959434012,
     0.1336826525834396151, 0.1386237799845948804, 0.1436100800906280339,
     0.1486415742423425057, 0.1537183122081819397, 0.1588403711394795748,
     0.1640078546834206341, 0.1692208922373653057, 0.1744796383307898324,
     0.1797842721232958407, 0.1851349970089926078, 0.1905320403191375633,
     0.1959756531162781534, 0.2014661100743140865, 0.2070037094399269362,
     0.2125887730717307134, 0.2182216465543058426, 0.2239026993850088965,
     0.229632325232116602, 0.2354109422634795556, 0.2412389935454402889,
     0.2471169475123218551, 0.2530452985073261551, 0.2590245673962052742,
     0.2650553022555897087, 0.271138079138385224, 0.2772735029191887857,
     0.2834622082232336471, 0.2897048604429605656, 0.2960021568469337061,
     0.3023548277864842593, 0.3087636380061818397, 0.3152293880650116065,
     0.3217529158759855901, 0.3283350983728509642, 0.3349768533135899506,
     0.3416791412315512977, 0.3484429675463274756, 0.355269384847918035,
     0.3621594953693184626, 0.3691144536644731522, 0.376135469510563536,
     0.3832238110559021416, 0.3903808082373155797, 0.3976078564938743676,
     0.404906420807223999, 0.4122780401026620578, 0.4197243320495753771,
     0.4272469983049970721, 0.4348478302499918513, 0.4425287152754694975,
     0.4502916436820402768, 0.458138716267873114, 0.4660721526894572309,
     0.4740943006930180559, 0.4822076463294863724, 0.4904148252838453348,
     0.4987186354709807201, 0.5071220510755701794, 0.5156282382440030565,
     0.5242405726729852944, 0.5329626593838373561, 0.5417983550254266145,
     0.5507517931146057588, 0.5598274127040882009, 0.5690299910679523787,
     0.5783646811197646898, 0.5878370544347081283, 0.5974531509445183408,
     0.6072195366251219584, 0.6171433708188825973, 0.6272324852499290282,
     0.6374954773350440806, 0.6479418211102242475, 0.6585820000500898219,
     0.6694276673488921414, 0.6804918409973358395, 0.6917891434366769676,
     0.7033360990161600101, 0.7151515074105005976, 0.7272569183441868201,
     0.7396772436726493094, 0.7524415591746134169, 0.7655841738977066102,
     0.7791460859296898134, 0.7931770117713072832, 0.8077382946829627652,
     0.8229072113814113187, 0.8387836052959920519, 0.8555006078694531446,
     0.873243048910072206, 0.8922816507840289901, 0.9130436479717434217,
     0.9362826816850632339, 0.9635996931270905952, 1}};

}  // namespace random_internal
ABSL_NAMESPACE_END
}  // namespace absl

// clang-format on
// END GENERATED CODE
