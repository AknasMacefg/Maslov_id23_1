/*
 * keystore cracker patch for JtR, common code. 2016 by JimF
 * This file takes replicated but common code, shared between the CPU
 * and the GPU formats, and places it into one common location
 */
#include "arch.h"
#include "misc.h"
#include "common.h"
#include "formats.h"
#include "keystore_common.h"
#include "loader.h"
#include "memdbg.h"

static int MAX_SALT_SIZE=819200;

struct fmt_tests keystore_common_tests[] = {
	{"$keystore$0$2126$feedfeed000000020000000100000001000f616e64726f696464656275676b65790000013c3ea72ab000000501308204fd300e060a2b060104012a021101010500048204e9e76fea55eed58e4257c253b670948abb18093fbbb667f00807560242f17a4b3cd8b90d0e2a5c6c96f758f45e0e2320039c10af4ecc95e56930fd85713318da506bb48fa586b5caf7c286cf3b66134cb0e13dcdbc665680fb1214d9db2405ccb297acdefd4f5f7cb1c1babd5b77414223b45ae11ab0ec0a2ce5423a6ab69f372adb79a38973a0fde89f9b1e8ef62de04a5e6b35008ce3191c350f98a98ed917ccfc3524f9a4786a3ab055cee25efb118f67d73cacfdd5a3f0ca04399d2b31acfffc63ab6b47f371ff879768ef84bc8c58bcfaab1539e6343cf7b81d0446f57abbeb84fb20b540616aabbfd4c823acb2124ea25538c7531609b72b8da90327a8a3845bcfd69d659a1a77c35efb0d62651e4178459dfde9e165edc6d52cc3d8fee78e3132346588b09e3d27e1400421d33e88748ed1c01af1dc6064a71c991e0322e72c55ed5bcd8c232048bddfecd299d4d9c296639866dd21ad073a4993733b44bac4d6a77eec05cda65d5d9ad0a42a5aa9d443e3ba7ea5744e7fdc2617f527cd9cf480bce033bd5eec6746b2a58328aeed26757664109e1046c93e2377db18c58c35828916f4a42964aae2fe75ad944896bd321ae92cd5723735b37f85250a635a8d1875d3efb2ffbcabc3602ea3b6952da060ec1d1c0a961b1a50836dee911a166e09a33d036d6ef7dc988545b580841945a8718b178bb06ef8e78c6703a496cf66990d57b696b2117922ee1855dff439b2bda3201b145fdb4533b7d2cfa22291a79bac67bb6b3d963dd4137b6208931f02c3ee30bfd0731443edadd5bfffec0147f5f2bd13930deace26fec0ebf0c1befe1294875fb9d8a08919fdc1697ec78d1b86c03a0db4e61bd6a9db6803fdd8e2547ead44bd48cf223b964b0c6903ede0fc0e1b7d02b83ba18ed649bc0e40896ff7cde1d092a9f30314da8fc67d113c79fe7046da75bc090b08b3f31a5d0feb33abab2c608e3afaca1521f2809ae79c14e5ab16d7fa319ddc4dbae61cf41bd15829055970f26361fc1ae22a15e401b25eb500411e70a3cacca38e0d59a6add6513c02d0e6a766303e231d8adf8368b1579e7d58a7d3a5981542c9b8fec0b1780713031fefa60d93755215cbbc34f27634537b6c4fe391578be1a3547fc97d1eeb3e8b11444e8ad99902911fba55034a2796d791039bb29bd193406f05b942f69d47a4a236a64f610e7808387586f4a96a84059e93b11355ecd9125e7a805503e41f4097893b043c7d539d76933515c8fbde11f2a69a6f47aebbac3ed29b0231b3a74ecc9a5421ad61c995a039e44c0a8717dd6e5efbdc2f6ab8daefbc58867ca2e852780c66d1163a03662c34b5365983405093452bb004f78eb973a804edb1b4e8214ab982ed9c81992cc508d8852288fee4ced3af41cca7baaddb828830f3e7dd7c92610def60bbaf6a866e84ea81bd4e88a5b5a035b15b370f942af17f213706c681a59da20b150697c188edb4ac8b59b3babf9c895078f268940aa805c15a2712042c22ce5c44a62554d5f2efb6db179e1db29570b6b063d00349a0273277751e6adf32b6d36b02cb81025d80e620b61a418b0584441c087ce75ed03c871dfe8463a9a3641b036e849fd0fdc9b381ebe43e067353642f182d67ef6bef43463dc6b8d7abd035677b443440c7624d91baa11002e193d86a76974eef4f6fb44a8c440b73ddb323e9eb8f7fdd67aa368ce6aefdff1060e6a519d48b28718b1548e4665360f141d5e16027f0e7c41d07c582dd2a29fa55a00f000000010005582e353039000003113082030d308201f5a00302010202043e310348300d06092a864886f70d01010b05003037310b30090603550406130255533110300e060355040a1307416e64726f6964311630140603550403130d416e64726f6964204465627567301e170d3133303131353134343030385a170d3133303431353134343030385a3037310b30090603550406130255533110300e060355040a1307416e64726f6964311630140603550403130d416e64726f696420446562756730820122300d06092a864886f70d01010105000382010f003082010a02820101009116fccceb121c8951fb12992ef59393a8c2aeab4ec76a30a71d3aca40a277ab1500613c30bda5472bc15812bdbe9395b4a6009edaf94ca7427cd94ca840c0ac9d42ab8246a401628dbba7acb408738929b75f319d496e8594afd75423c07299ec195efce351b7f2b730ad5e61ab292a4783611cdad41139302ada3e239656c2ec842a59418efc711072e75193cfba1105a1980a631f4a513e4116a89806a47f8b308c03684e2ce83e03c40c438445143fa3fab756909e101f89410a35bb6e6a5cbdcef19d0359c8ed7862fe7ae7f81c32a9a75f72419f89eddbe4acc4373e45a390fd185ae3b28adb8445c4e38e30773acad396788428b0321936f241e905c50203010001a321301f301d0603551d0e041604148c2df598ae53bebe11c4e4696abc6cad6bce4286300d06092a864886f70d01010b05000382010100507e62f723154b2e818140fbc47547c8a600f97a580de244afdf6cdc02977aa7fb990c77a0d79d3ef53aadcf9d7705b385c365e3e06bf15de1a9d3f5c6b6b40fc4b629f763da8f12fc16a005b66026de2be8f1144d37ef14fc1c99dc13dd33fc750898a7ac9e2a12543402ba5021432a8453d38b4879a95736f65956d13d92d96b6f546b853c92f0cc51a98dcd233076ae285d5ed44601f1fe361974c74067eb263386fe8e085e8b20c3cd72768d4265bd9bf4937b2aeae3323c6289dfe75e820907ba38e85b3fc2ceb44e770b91babfdf1d003bbc56ed7066f97ba86e0648ff0874a31c1563d52f42f38005b3698f800be11257f405b185ca421113072f8531$a8ab7a46059faddb183f66d4aef78f47911c88aa$1$1281$308204fd300e060a2b060104012a021101010500048204e9e76fea55eed58e4257c253b670948abb18093fbbb667f00807560242f17a4b3cd8b90d0e2a5c6c96f758f45e0e2320039c10af4ecc95e56930fd85713318da506bb48fa586b5caf7c286cf3b66134cb0e13dcdbc665680fb1214d9db2405ccb297acdefd4f5f7cb1c1babd5b77414223b45ae11ab0ec0a2ce5423a6ab69f372adb79a38973a0fde89f9b1e8ef62de04a5e6b35008ce3191c350f98a98ed917ccfc3524f9a4786a3ab055cee25efb118f67d73cacfdd5a3f0ca04399d2b31acfffc63ab6b47f371ff879768ef84bc8c58bcfaab1539e6343cf7b81d0446f57abbeb84fb20b540616aabbfd4c823acb2124ea25538c7531609b72b8da90327a8a3845bcfd69d659a1a77c35efb0d62651e4178459dfde9e165edc6d52cc3d8fee78e3132346588b09e3d27e1400421d33e88748ed1c01af1dc6064a71c991e0322e72c55ed5bcd8c232048bddfecd299d4d9c296639866dd21ad073a4993733b44bac4d6a77eec05cda65d5d9ad0a42a5aa9d443e3ba7ea5744e7fdc2617f527cd9cf480bce033bd5eec6746b2a58328aeed26757664109e1046c93e2377db18c58c35828916f4a42964aae2fe75ad944896bd321ae92cd5723735b37f85250a635a8d1875d3efb2ffbcabc3602ea3b6952da060ec1d1c0a961b1a50836dee911a166e09a33d036d6ef7dc988545b580841945a8718b178bb06ef8e78c6703a496cf66990d57b696b2117922ee1855dff439b2bda3201b145fdb4533b7d2cfa22291a79bac67bb6b3d963dd4137b6208931f02c3ee30bfd0731443edadd5bfffec0147f5f2bd13930deace26fec0ebf0c1befe1294875fb9d8a08919fdc1697ec78d1b86c03a0db4e61bd6a9db6803fdd8e2547ead44bd48cf223b964b0c6903ede0fc0e1b7d02b83ba18ed649bc0e40896ff7cde1d092a9f30314da8fc67d113c79fe7046da75bc090b08b3f31a5d0feb33abab2c608e3afaca1521f2809ae79c14e5ab16d7fa319ddc4dbae61cf41bd15829055970f26361fc1ae22a15e401b25eb500411e70a3cacca38e0d59a6add6513c02d0e6a766303e231d8adf8368b1579e7d58a7d3a5981542c9b8fec0b1780713031fefa60d93755215cbbc34f27634537b6c4fe391578be1a3547fc97d1eeb3e8b11444e8ad99902911fba55034a2796d791039bb29bd193406f05b942f69d47a4a236a64f610e7808387586f4a96a84059e93b11355ecd9125e7a805503e41f4097893b043c7d539d76933515c8fbde11f2a69a6f47aebbac3ed29b0231b3a74ecc9a5421ad61c995a039e44c0a8717dd6e5efbdc2f6ab8daefbc58867ca2e852780c66d1163a03662c34b5365983405093452bb004f78eb973a804edb1b4e8214ab982ed9c81992cc508d8852288fee4ced3af41cca7baaddb828830f3e7dd7c92610def60bbaf6a866e84ea81bd4e88a5b5a035b15b370f942af17f213706c681a59da20b150697c188edb4ac8b59b3babf9c895078f268940aa805c15a2712042c22ce5c44a62554d5f2efb6db179e1db29570b6b063d00349a0273277751e6adf32b6d36b02cb81025d80e620b61a418b0584441c087ce75ed03c871dfe8463a9a3641b036e849fd0fdc9b381ebe43e067353642f182d67ef6bef43463dc6b8d7abd035677b443440c7624d91baa11002e193d86a76974eef4f6fb44a8c440b73ddb323e9eb8f7fdd67aa368ce6aefdff1060e6a519d48b28718b1548e4665360f141d5e16027f0e7c41d07c582dd2a29fa55a00f", "android"},
	{"$keystore$0$2195$feedfeed00000002000000010000000100036a747200000152c3517c8300000502308204fe300e060a2b060104012a021101010500048204eae4df1714611cfa0c85393be60f0e15e3ad5fb8798dc721ef2213cae24d07c17851b25974b69dbf24e1b500297f5b176795d592652e6d9a1c1cb45e94385993302f285eca7452b3e7605b0ff8a54c23d27e664036e22a56f809ffaef7bd2b56ba959ab90854c900149ee70ebcceccc8cb22b07e3cdc7376ffd8280e21a8341d5cc37b460be100e6936514f112d5bad07d114f324061402be35260e249773c454487b5cf44824a827506ce5e08accff26dfca30f719bdc6909ff0cb91e6b490258efbf87e0ec07b46ec2279ca4bf04f74fd4fa64613415095a0f532c89e7e238e24e1dbafc6351f4573473cbac84d084686bf9d17bdc1e4e2f46c574e1f4699e6789f86a29b4b1c90d3fe03a70c7cc60afca259a54d8ed3daf0422b410733a766bae845ad6ad966d83ee31f63f6e1aaac473f2f3926b0ea859dc9246a023d605161473b2bf3952cf4aea9653ebc5da15bc693811776797a32d9b7f4c72585a340b8e960a980b362f78bcd51af3bb6995ac02ba08188d9009df2c1cd17d819b8704929eac1344c0442c957597586bb04acc7a45dd10a54108c07a72403a36bc631fec257c8653948464eb25ce05a21173c47af7774625ed3fc9103ac24ba5522ed714b812c4d9006a9da58f992743fbe2910d15c66b2263fe109618419ab9d78f374daf2f03b89aad87a16ceaa547778f4e48dc6c4ea43b7ef0704fc3992a2459f8c3b416f0c416bd96d1b5fcaf8b4fabadbae8581e6d8b9906017d7df72934e4f7a2c2409432da3a97b8a6324f0e38165511262c8c92cff54a7ede63fab78c2cf7bcc0b78111b40fe974619b5e6b7e71fd100ac79845220b0117ffc1052c44f315554204ee8d271226529a6e35054adc5bbf5bcd822491e517bc73acd86cbb9ce2099352d0f63245ec9dbf5ba97622ec920318a56e39d124cb2d35744e9cf8517e32714d66a5ac17df495edd08c91b77cc8e54653a8766229885864aebd5f6db61bec4d4948c31618510b98b1d2fdf50d15de2b679ff647e396d11df5a9b67b20e70e3fdd5913ec1665eaeed792a422302202009964021bf5dcb71689c12875a51a4e5fd0a735752d815ee2bb18193aeeca800d8a532454abe288380d9a29768a162898bc0d116d4c743fb4cebaf065ac984e66e47fd3321f9c77423a5e9d1a5afc11ff8a4dcf877ce9aed557e6e1460451c565350c0a1a6cec71e5f7c9ac0aa3f6a037cafc8d54bb688f89cecea9ed14241c273725071924435c11cbaa9a0bd9d2ccffd90870f16a7ebb7fe6ac5bb7cfcfa5b038510180f5c6e3ec44978721006b02a29e40f4e12402a6a5fb17e96c0afbbcc3ca854dd9143dec67012ff9ce38f06a635926abf58dcd831b2791e8428d184975bd6ac5fcbb103afa1b4582b33a6a675ca7401ed0580e89172a5fdbfb44acd79d754ad9ac280905038535ebbf1fe2a43b25af1e1ef52e4dcb707e6f2a6bfee51b9c64b4f46e61083fca64bee56fbcd70a1fcf651aaf08d6397862697060467e94ed25d5d9f446f6113d3d632d979e597958bd38a41ea7b7ebeeb6ce8dfba4c84ab8c20938fa9cfd7f3e2231ecd766500fbcf422f99eac0857eef345f8e08c25508db460e3911e98bac032489c6b3a29c5b15e2a9acf7f6fefa1c681ef1c4f3c89b59027d85f6f66bae955d0e6ba93a83876d2c262b0ea87d0fa96130df6cb87b66c7f859579fae6ef8921c4e901dae716391c65b7da157e401887f002999dd7d069ad8ef753c20f2ff9a73fc0ed0b32dea9ef9bbe5328885000000010005582e353039000003613082035d30820245a00302010202044e462f7b300d06092a864886f70d01010b0500305f310b3009060355040613025255310f300d06035504081306626974656d653110300e060355040713076e6f7768657265310d300b060355040a13046e6f6e65310d300b060355040b130473656c66310f300d06035504031306626f6f626f6f301e170d3136303230383233353731365a170d3136303530383233353731365a305f310b3009060355040613025255310f300d06035504081306626974656d653110300e060355040713076e6f7768657265310d300b060355040a13046e6f6e65310d300b060355040b130473656c66310f300d06035504031306626f6f626f6f30820122300d06092a864886f70d01010105000382010f003082010a028201010091dedb89bd398bdf496ac2956920609e8e616101abd0335d2ff972cc6a9c91f555a25f2d10d33213cf4f68288a372fa1b4a254a624b58a0bc0c4bb178e2b0e1308e2cf05b8abad4240f5e8a3c3d0c7d83ff1f53972c70ee5c118df17c6f99c70d0949e0b6296d83b28bafa9c799dc3b2d00f3f2cda4b25f094e7330d45f1aa57976179ed9284b20bf4d3ed659b94bd8ec7388ddae93d11819b61fd191876fb615d73ed4ef9f4d5fd940ad7032104c484f7f99d71de98f9e312fbbf08b676fd5488585697dd8d188238624281dafd04a34e9dbe22a94e8200a418d570c32507cdc95c477c20d64a10d12fc5fecf76d73dad7f7fd01de31ba1b7af571b8d11538b0203010001a321301f301d0603551d0e041604146dc6d89fc9d145f1d2bf432a86769b1ef6afd9c2300d06092a864886f70d01010b050003820101008ea9abedd162507358a1efc87d38c350380f47bc777db9d77991d0e20ad37bdc1bdeb2e29803714aecf325acd4db60d0863d3c3c7b5984ae2bd90641ff7407c64abc7ec857d6325aef4e39d9ea6c68cfd0be3122c75bafd39c4327fc7897e5970f6ee3f91b002bbedb5dc82cb035d562b86b769c94f8ecf11bd5be0600f0ea63694dc71372100ce59b8b5318133c79f5e51d1efd3f2be1df4af9b62eb29e4ac02f6a57ec19fd9a3f5b05e4555abfd39357acb484562497d7a8244ce6ad442b0cba7f0fd4a99e200964514a4add10bb699a40da49f95730f13df9732abec8d1d628bafe9a0c1fe4d161fbec398d2d75e7ce5a31afc52bb10f6d4fced938cd2e0d$1ee249c5b30cc96c84326d6cec7a11f7f46d626d$1$1282$308204fe300e060a2b060104012a021101010500048204eae4df1714611cfa0c85393be60f0e15e3ad5fb8798dc721ef2213cae24d07c17851b25974b69dbf24e1b500297f5b176795d592652e6d9a1c1cb45e94385993302f285eca7452b3e7605b0ff8a54c23d27e664036e22a56f809ffaef7bd2b56ba959ab90854c900149ee70ebcceccc8cb22b07e3cdc7376ffd8280e21a8341d5cc37b460be100e6936514f112d5bad07d114f324061402be35260e249773c454487b5cf44824a827506ce5e08accff26dfca30f719bdc6909ff0cb91e6b490258efbf87e0ec07b46ec2279ca4bf04f74fd4fa64613415095a0f532c89e7e238e24e1dbafc6351f4573473cbac84d084686bf9d17bdc1e4e2f46c574e1f4699e6789f86a29b4b1c90d3fe03a70c7cc60afca259a54d8ed3daf0422b410733a766bae845ad6ad966d83ee31f63f6e1aaac473f2f3926b0ea859dc9246a023d605161473b2bf3952cf4aea9653ebc5da15bc693811776797a32d9b7f4c72585a340b8e960a980b362f78bcd51af3bb6995ac02ba08188d9009df2c1cd17d819b8704929eac1344c0442c957597586bb04acc7a45dd10a54108c07a72403a36bc631fec257c8653948464eb25ce05a21173c47af7774625ed3fc9103ac24ba5522ed714b812c4d9006a9da58f992743fbe2910d15c66b2263fe109618419ab9d78f374daf2f03b89aad87a16ceaa547778f4e48dc6c4ea43b7ef0704fc3992a2459f8c3b416f0c416bd96d1b5fcaf8b4fabadbae8581e6d8b9906017d7df72934e4f7a2c2409432da3a97b8a6324f0e38165511262c8c92cff54a7ede63fab78c2cf7bcc0b78111b40fe974619b5e6b7e71fd100ac79845220b0117ffc1052c44f315554204ee8d271226529a6e35054adc5bbf5bcd822491e517bc73acd86cbb9ce2099352d0f63245ec9dbf5ba97622ec920318a56e39d124cb2d35744e9cf8517e32714d66a5ac17df495edd08c91b77cc8e54653a8766229885864aebd5f6db61bec4d4948c31618510b98b1d2fdf50d15de2b679ff647e396d11df5a9b67b20e70e3fdd5913ec1665eaeed792a422302202009964021bf5dcb71689c12875a51a4e5fd0a735752d815ee2bb18193aeeca800d8a532454abe288380d9a29768a162898bc0d116d4c743fb4cebaf065ac984e66e47fd3321f9c77423a5e9d1a5afc11ff8a4dcf877ce9aed557e6e1460451c565350c0a1a6cec71e5f7c9ac0aa3f6a037cafc8d54bb688f89cecea9ed14241c273725071924435c11cbaa9a0bd9d2ccffd90870f16a7ebb7fe6ac5bb7cfcfa5b038510180f5c6e3ec44978721006b02a29e40f4e12402a6a5fb17e96c0afbbcc3ca854dd9143dec67012ff9ce38f06a635926abf58dcd831b2791e8428d184975bd6ac5fcbb103afa1b4582b33a6a675ca7401ed0580e89172a5fdbfb44acd79d754ad9ac280905038535ebbf1fe2a43b25af1e1ef52e4dcb707e6f2a6bfee51b9c64b4f46e61083fca64bee56fbcd70a1fcf651aaf08d6397862697060467e94ed25d5d9f446f6113d3d632d979e597958bd38a41ea7b7ebeeb6ce8dfba4c84ab8c20938fa9cfd7f3e2231ecd766500fbcf422f99eac0857eef345f8e08c25508db460e3911e98bac032489c6b3a29c5b15e2a9acf7f6fefa1c681ef1c4f3c89b59027d85f6f66bae955d0e6ba93a83876d2c262b0ea87d0fa96130df6cb87b66c7f859579fae6ef8921c4e901dae716391c65b7da157e401887f002999dd7d069ad8ef753c20f2ff9a73fc0ed0b32dea9ef9bbe5328885", "johnripper"},
	// this keystore was made with a utf-8 password. NOTE, this was made with keytool from jdk1.7.0_45
	{"$keystore$0$2197$feedfeed000000020000000100000001000c6f70656e77616c6c2e6e657400000152cea5402000000501308204fd300e060a2b060104012a021101010500048204e9e0416520c63207fa6c76c56048c798b032b593ea9cf5d8a68dfad92f27ab3b948e5b0e48444f5febd31c659add409aca52220132b760a7def534908c4342f81be032e48f08673299ce6ceb67b398676cf39f95e4099f85bf430aee96c3429562b74937f29ffb889e5c760d1d4b6fb8a8e98dcef6c1f96df0573065383e3805fa5e2982d772bd34bdd1200c2fcdb5fd315a2eaf0f4c5076848a6a5f7ba2407e252ed30bb5c189cf0b5ece95cffa7da9e1b07fd6cd954f78f929bb3fc00481d9e879a7e771adb0c8193081342ff4ffb057fd0699cfa9147cee4a1581136b1ea7f28dbd84090484c8dbcdb961c38c8d1fb281c2abb2690a5abf083c317c0068fb7232db45a0097ae4dddf0575a50aacebf6a457d40a2c9599e1d25c3f69ba0fae516b29dcfe51b7a9dfebc31ac224ba3753674794bdaae335e8172a30daf06655c9ae4cd132703f42f8edc6946f7e36b012c7ef895d048cc9fefe539d9034113ef1d9d4b652b21ce42fd95ba9e8b32ce44083d961f8ba62d77a1eca4973342f026d9150bcd16354e7294074404741fa2145ce3209f1cb96bdaa2585d49495a85b1e9ea5da521ac584175bf7caff15a002e3f81b06d136b199ede817f12ed299f566a65a357d5c21b1ccdbb6edc857b800e70144490b4a07ea07e706e0679d26abcfa12217948a6820c0230916cb8215c486e60f99e79d8e202478987c0b94d7c065f8d8c4d6f78372a30be7f3f5980dd4f979fbce8e49211b0e62d11db71928ee2dad61926fac9e814dcfd0ff616ea3f61545cc17af0fb53fd2e39215e755c2422dba0d43e926f9132229d1f8f821f8dc7a95579fa7f19686e5b21378cf6042346551a2f5095d414cfa35f52ee9393e869df9887bba29dcec691f99840e2ce801009654db01ffc20bfb8320b91c43b1971371041319336003391e5950f0093ccc7379b8d44bde0cda5b786020b2ba01f98a21c579c5f5088b622fbd507c9177d8f0c82656d3ae1578b0c7ed957845ab788a37363d4e438071a99a2cd4052f6f8fe7c2d200f6fd2d35a408c5b9441f077fb983ba6fde895bdcdcfd4050c1e586db64546458d83f7be37308f7b43890d2f7befce7d8247e733d3b493040c8b0d228a37e54fc8f1becd317fd330b47590a4e818fea148671441248f754c1b7fb9cf0191092064dcbbf75df4c2d69be372970d99ab41cfd6668fa7baeb4f9c0130d098cceb33c37eb2cba7ca42ca8bad96f6185868f49c20829d9873220aed9c41f708bdc38d19692bb47ae7a16634364371194475334cabdecbe1a377145ce1b6931b108c4efcb3138f75bb1cea18dd1c56a552d1291144c910ea885165a57e8d3159d590b5c50cd135a661692e7285b58ed2559a7c55d1f0c0fcd3fd275865f800641c2f79377d16c6ed10976035bd6cb15ee6942f2ea3fee2f30939eebd0915535c8cda3670f641c7654942bf02dc96ac42b9fa9de3b5ce3e84697612ebafc2d21c26509a6e3ae33bea8d6728e41a7208c7f858840dfdcf5c573693492f946538e5f982a6e21b1efaa329a11c825e64117a73ff6f30955a197bfb57985abec1397cbe70eb27e635b1d7c605d1916b68aefb951936290ada33be9f9c9dc8a9ed05ad38e930f1f6e0b8fc959a2556b4d34dae07aae9aa6a0f1c80cc7fdd97046ff3889bc21d1bee73b82980b743d2b9c5e31e8ce15edeff384556f09e102dc5168c069fad7518b3b6ffa7a58c920d5e22b6172fa06a78d63cbb654766de15c54b9f54b30d43ac7c0b7210aa8000000010005582e3530390000035b308203573082023fa0030201020204406dbc70300d06092a864886f70d01010b0500305c310b3009060355040613024655310b3009060355040813024655310d300b060355040713044655204f3111300f060355040a13086f70656e77616c6c310d300b060355040b13046861636b310f300d060355040313066e6f206f6e65301e170d3136303231313034343433375a170d3136303531313034343433375a305c310b3009060355040613024655310b3009060355040813024655310d300b060355040713044655204f3111300f060355040a13086f70656e77616c6c310d300b060355040b13046861636b310f300d060355040313066e6f206f6e6530820122300d06092a864886f70d01010105000382010f003082010a028201010090446c1a80dbd5728683aab36ebe5e0419a65edd43b63ba18064a7612dcc22b589934deeab63a4b1998e1244100e32c2b5db8fc45fe0cf8c040d2fae9b19982bfa0bc0e52c03f439e4d676f3509a37141223e4216b5d55ea4be35f3e49941fc7c80859898125cd33cfe523054808716e82f101c1a2655b714284d49fda700cd9ae91ad9a879f9e356f9c1c346978b49f3eb1367bca2abd6d3eb0993d1fce33ec5961323309ee2a8730ab703fc19689d98574120aa32581fa70ae9462936d0a5cfdf9359eada74a01fd44f6e3418756211ba67ef9f7b334b8cf27d1139e4f49ff6ba23d56f3d25ded14475681d53e23e8c68c487dff5e588149b0cd1f719e852f0203010001a321301f301d0603551d0e04160414410470c17666448406500afabfeef5e425b874d4300d06092a864886f70d01010b050003820101006dee4f34979e0bc5366aef9b24e4c51da82600391105f523aefa0b8656af350964ee3f82df9fb8b3b38768bbb3e9aab103ca61198d2a50476c7fa23e8fd1f1bf1f066b132592940a8347998af52c40e647a6a269e53b9e2f29ffe56aee5e1a9d83110552efb633e5c4c2acd57ba403a4d62244c0e6eb077ed0a2ac50d41e2e22998d2f0fc0158faefda27574e9c1e7302f3dc0896b61b62bf269de79df4c66703ec7adc5885cc0dc385992bdea6ebacb0e5091922daba90c60bd2062d381b288a9c99d588d45effd88e2639b35847a1301a26aeae483d272d6ee5a6844d9bb2785b2aae579f15073bbded3cc7a9df8c371f778f120e4f0dc0b72107ccac0d103$c8ab39fab0cb260ea85fd495bfc48a60de8e9fa1$1$1281$308204fd300e060a2b060104012a021101010500048204e9e0416520c63207fa6c76c56048c798b032b593ea9cf5d8a68dfad92f27ab3b948e5b0e48444f5febd31c659add409aca52220132b760a7def534908c4342f81be032e48f08673299ce6ceb67b398676cf39f95e4099f85bf430aee96c3429562b74937f29ffb889e5c760d1d4b6fb8a8e98dcef6c1f96df0573065383e3805fa5e2982d772bd34bdd1200c2fcdb5fd315a2eaf0f4c5076848a6a5f7ba2407e252ed30bb5c189cf0b5ece95cffa7da9e1b07fd6cd954f78f929bb3fc00481d9e879a7e771adb0c8193081342ff4ffb057fd0699cfa9147cee4a1581136b1ea7f28dbd84090484c8dbcdb961c38c8d1fb281c2abb2690a5abf083c317c0068fb7232db45a0097ae4dddf0575a50aacebf6a457d40a2c9599e1d25c3f69ba0fae516b29dcfe51b7a9dfebc31ac224ba3753674794bdaae335e8172a30daf06655c9ae4cd132703f42f8edc6946f7e36b012c7ef895d048cc9fefe539d9034113ef1d9d4b652b21ce42fd95ba9e8b32ce44083d961f8ba62d77a1eca4973342f026d9150bcd16354e7294074404741fa2145ce3209f1cb96bdaa2585d49495a85b1e9ea5da521ac584175bf7caff15a002e3f81b06d136b199ede817f12ed299f566a65a357d5c21b1ccdbb6edc857b800e70144490b4a07ea07e706e0679d26abcfa12217948a6820c0230916cb8215c486e60f99e79d8e202478987c0b94d7c065f8d8c4d6f78372a30be7f3f5980dd4f979fbce8e49211b0e62d11db71928ee2dad61926fac9e814dcfd0ff616ea3f61545cc17af0fb53fd2e39215e755c2422dba0d43e926f9132229d1f8f821f8dc7a95579fa7f19686e5b21378cf6042346551a2f5095d414cfa35f52ee9393e869df9887bba29dcec691f99840e2ce801009654db01ffc20bfb8320b91c43b1971371041319336003391e5950f0093ccc7379b8d44bde0cda5b786020b2ba01f98a21c579c5f5088b622fbd507c9177d8f0c82656d3ae1578b0c7ed957845ab788a37363d4e438071a99a2cd4052f6f8fe7c2d200f6fd2d35a408c5b9441f077fb983ba6fde895bdcdcfd4050c1e586db64546458d83f7be37308f7b43890d2f7befce7d8247e733d3b493040c8b0d228a37e54fc8f1becd317fd330b47590a4e818fea148671441248f754c1b7fb9cf0191092064dcbbf75df4c2d69be372970d99ab41cfd6668fa7baeb4f9c0130d098cceb33c37eb2cba7ca42ca8bad96f6185868f49c20829d9873220aed9c41f708bdc38d19692bb47ae7a16634364371194475334cabdecbe1a377145ce1b6931b108c4efcb3138f75bb1cea18dd1c56a552d1291144c910ea885165a57e8d3159d590b5c50cd135a661692e7285b58ed2559a7c55d1f0c0fcd3fd275865f800641c2f79377d16c6ed10976035bd6cb15ee6942f2ea3fee2f30939eebd0915535c8cda3670f641c7654942bf02dc96ac42b9fa9de3b5ce3e84697612ebafc2d21c26509a6e3ae33bea8d6728e41a7208c7f858840dfdcf5c573693492f946538e5f982a6e21b1efaa329a11c825e64117a73ff6f30955a197bfb57985abec1397cbe70eb27e635b1d7c605d1916b68aefb951936290ada33be9f9c9dc8a9ed05ad38e930f1f6e0b8fc959a2556b4d34dae07aae9aa6a0f1c80cc7fdd97046ff3889bc21d1bee73b82980b743d2b9c5e31e8ce15edeff384556f09e102dc5168c069fad7518b3b6ffa7a58c920d5e22b6172fa06a78d63cbb654766de15c54b9f54b30d43ac7c0b7210aa8", "\xce\x9c\xce\xb1\xce\xb3\xcf\x8e\xce\xb3"},
	{NULL}
};

MAYBE_INLINE static int keystore_common_valid(char *ciphertext, struct fmt_main *self)
{
	char *p;
	char *ctcopy;
	char *keeptr;
	int target;
	int v, extra;
	if (strncmp(ciphertext, FORMAT_TAG, FORMAT_TAG_LEN) != 0)
		return 0;
	ctcopy = strdup(ciphertext);
	keeptr = ctcopy;
	ctcopy += FORMAT_TAG_LEN;
	if ((p = strtokm(ctcopy, "$")) == NULL)
		goto bail;
	if (!isdec(p))
		goto bail;
	target = atoi(p);
	if (target != 1 && target != 0)
		goto bail;
	if ((p = strtokm(NULL, "$")) == NULL)
		goto bail;
	if (!isdec(p))
		goto bail;
	v = atoi(p);
	if ((p = strtokm(NULL, "$")) == NULL)
		goto bail;
	if (hexlenl(p, &extra) != v*2 || extra)
		goto bail;
	if ((p = strtokm(NULL, "$")) == NULL) /* hash */
		goto bail;
	if (hexlenl(p, &extra) != BINARY_SIZE*2 || extra)
		goto bail;
	if ((p = strtokm(NULL, "$")) == NULL) /* number of keys */
		goto bail;
	if (!isdec(p))
		goto bail;
	/* currently we support only 1 key */
	if(atoi(p) != 1)
		goto bail;
	if ((p = strtokm(NULL, "$")) == NULL) /* key length */
		goto bail;
	if (!isdec(p))
		goto bail;
	v = atoi(p);
	if (v > MAX_SALT_SIZE)
		goto bail;
	if ((p = strtokm(NULL, "$")) == NULL) /* key data */
		goto bail;
	if (hexlenl(p, &extra) != v*2 || extra)
		goto bail;
	MEM_FREE(keeptr);
	return 1;
bail:
	MEM_FREE(keeptr);
	return 0;
}

int keystore_common_valid_gpu(char *ciphertext, struct fmt_main *self) {
	MAX_SALT_SIZE = SALT_LENGTH_GPU;
	return keystore_common_valid(ciphertext, self);
}
int keystore_common_valid_cpu(char *ciphertext, struct fmt_main *self) {
	MAX_SALT_SIZE = SALT_LENGTH_CPU;
	return keystore_common_valid(ciphertext, self);
}

void *keystore_common_get_binary(char *ciphertext)
{
	static union {
		unsigned char c[BINARY_SIZE];
		ARCH_WORD dummy;
	} buf;
	unsigned char *out = buf.c;
	int i;
	char *ctcopy = strdup(ciphertext);
	char *keeptr = ctcopy;
	char *p;

	ctcopy += FORMAT_TAG_LEN; /* skip over "$keystore$" */
	p = strtokm(ctcopy, "$");
	p = strtokm(NULL, "$");
	p = strtokm(NULL, "$");
	p = strtokm(NULL, "$"); /* at hash now */

	for (i = 0; i < BINARY_SIZE; i++) {
		out[i] =
		    (atoi16[ARCH_INDEX(*p)] << 4) |
		    atoi16[ARCH_INDEX(p[1])];
		p += 2;
	}
	MEM_FREE(keeptr);
	return out;
}
